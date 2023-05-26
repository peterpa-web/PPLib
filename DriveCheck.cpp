#include "stdafx.h"
#include "Ping.h"
#include "SensAPI.h"	// IsNetworkAlive 
#include "EventLog.h"
#include "SimpleSocket.h"

#include "DriveCheck.h"

#pragma comment( lib, "SensAPI" ) 

CDriveCheck::CDriveInfo::CDriveInfo()
{
//	m_timeUpd = CTime::GetCurrentTime();
//	CTimeSpan d(600);
//	m_timeStart = m_timeUpd - d;
}

CDriveCheck::CDriveInfo::CDriveInfo(const CString& strName)
{
	m_strName = strName;
	m_strName.MakeLower();
	m_timeUpd = CTime::GetCurrentTime();
	CTimeSpan d(3600);	// -1h: avoid waiting startup share
	m_timeStart = m_timeUpd - d;
}

void CDriveCheck::CDriveInfo::SetWakeLanData(const CStringA& strWakeNetAddr, unsigned int mac[6])
{
	m_strWakeNetAddr = strWakeNetAddr;
	for (int n = 0; n < 6; ++n)
		m_mac[n] = mac[n];
}

void CDriveCheck::CDriveInfo::SetStatus(enum Status stat)
{
	int nSecs = 60;
	if (stat == Status::Starting)
		nSecs = 600;	// 10m
	else if (stat == Status::Online)
		nSecs = 3600;	// 1h
	else if (stat <= Status::Reset)
		nSecs = 0;
	CTimeSpan d(nSecs);
	TRACE3("CDriveInfo::SetStatus %d %s %ds\n", stat, (LPCTSTR)m_strName, nSecs);

//	if (stat > Status::Reset)
//		ASSERT(CTime::GetCurrentTime() >= m_timeUpd);
	if (stat != Status::Connecting)
		m_strNetConnStatus.Empty();
	m_timeUpd = CTime::GetCurrentTime() + d;
	m_status = stat;
}

constexpr auto MAGICPORT = "9";

bool CDriveCheck::CDriveInfo::WakeLan()
{
	TRACE0("CDriveCheck::CDriveInfo:: WakeLan\n");
	//	unsigned char mac[6] = { 0xCC, 0x5D, 0x4E, 0xC9, 0xFF, 0x40 };
		// 6x FF + 16x MACAddr (6Bytes)
	unsigned char buf[6 + 16 * 6];
	unsigned char* p = buf;

	if (m_strWakeNetAddr.IsEmpty())
		return false;

	for (int i = 0; i < 6; ++i)
		*p++ = 0xFF;

	for (int n = 0; n < 16; ++n) {
		for (int i = 0; i < 6; ++i)
			*p++ = (unsigned char)m_mac[i];
	}

	CUdpSocket sockSend;
	BOOL b = sockSend.Create(m_strWakeNetAddr, MAGICPORT, AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP);
	if (b)
		sockSend.SendTo(buf, 6 + 16 * 6, 0);
	sockSend.Close();
	return true;
}

bool CDriveCheck::CDriveInfo::NetConn()
{
	if (m_strShareName.IsEmpty())
		return true;
	if (m_strNetConnStatus == _T("ok"))
		return true;

	TRACE1("NetConn %s\n", m_strShareName);
	NETRESOURCE netrc;
	netrc.dwScope = RESOURCE_GLOBALNET;
	netrc.dwType = RESOURCETYPE_DISK;
	netrc.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	netrc.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	netrc.lpLocalName = NULL;
	netrc.lpRemoteName = (LPTSTR)(LPCTSTR)m_strShareName;
	netrc.lpProvider = NULL;

	DWORD dwErr = NO_ERROR;

	if (m_strUser.IsEmpty())
		dwErr = WNetAddConnection2(&netrc, NULL, NULL, 0);
	else
		dwErr = WNetAddConnection2(&netrc, m_strPasswd, m_strUser, 0);

	if (dwErr == NO_ERROR)
	{
		m_strNetConnStatus = _T("ok");
		return true;
	}
	if (dwErr == ERROR_BAD_NET_NAME || dwErr == ERROR_NETWORK_UNREACHABLE ||
		dwErr == ERROR_BAD_NETPATH)
	{
		m_strNetConnStatus = _T("Error: no connection to server ") + m_strShareName;
		TRACE1("No Connection to %s\n", m_strShareName);
		return false;
	}
	if (dwErr == ERROR_SESSION_CREDENTIAL_CONFLICT)	// 1219
	{
		TRACE1("ERROR_SESSION_CREDENTIAL_CONFLICT %s\n", m_strShareName);
//		dwErr = WNetCancelConnection2(netrc.lpRemoteName, 0, TRUE);
		m_strNetConnStatus = _T("Error: ") + m_strShareName + _T(" credential conflict: please re-login");
		return false;
//		if (dwErr == NO_ERROR)
//		{
//			TRACE0("CancelConn done\n");
//			return false;	// retry AddConn later
//		}
		// else set status below
	}
	if (dwErr == ERROR_ALREADY_ASSIGNED)		// 85
	{
		m_strNetConnStatus = _T("server ") + m_strShareName + _T(" ERROR_ALREADY_ASSIGNED");
		TRACE1("ERROR_ALREADY_ASSIGNED %s\n", m_strShareName);
		return false;
	}
	if (dwErr == ERROR_IO_PENDING)	// 997
	{
		m_strNetConnStatus = _T("server ") + m_strShareName + _T(" ERROR_IO_PENDING");
		TRACE1("ERROR_IO_PENDING %s\n", m_strShareName);
		return false;
		//		Sleep( 2000 );
		//		dwErr = NO_ERROR;	// no retry
	}
	if (dwErr == ERROR_LOGON_FAILURE)	// 1326
	{
		m_strNetConnStatus = _T("server ") + m_strShareName + _T(" ERROR_LOGON_FAILURE");
		TRACE1("ERROR_LOGON_FAILURE %s\n", m_strShareName);
		return false;
	}
	m_strNetConnStatus = CEventLogException::GetLastErrorText(dwErr);
	return false;
}

CString CDriveCheck::CDriveInfo::StatusMsg()
{
	if (IsRunning())
		return _T("");
	if (HasNoNet())
		return _T("Network is not alive.");
	if (HasNoPing())
		return _T("(Server \"") + m_strName + _T("\" is not alive.)");
	if (!m_strNetConnStatus.IsEmpty())
		return m_strNetConnStatus;
	if (IsStarting())
		return _T("(Waiting for server \"") + m_strName + _T("\".)");
	if (IsConnecting())
		return _T("(Server \"") + m_strName + _T("\" not connected.)");
	return _T("Server \"") + m_strName + _T("\" is not running.");
}

//##############################################################################


CDriveCheck::CDriveInfo& CDriveCheck::GetNetDriveInfo(CString strSrv)
{
	if (strSrv.Left(2) == _T("\\\\"))
	{
		int iStart = 2;
		strSrv = strSrv.Tokenize(_T("\\"), iStart);
	}
	strSrv.MakeLower();
	POSITION pos = m_listDriveInfo.GetHeadPosition();
	while (pos)
	{
		CDriveInfo& driveInfo = m_listDriveInfo.GetNext(pos);
		if (driveInfo.GetName() == strSrv)
		{
			TRACE2("CDriveCheck::GetNetDriveInfo %s valid for %ds\n", strSrv, driveInfo.GetValidSecs());
			return driveInfo;
		}
	}
	TRACE1("CDriveCheck::GetNetDriveInfo %s new\n", strSrv);
	CDriveInfo driveInfoNew(strSrv);
	pos = m_listDriveInfo.AddTail(driveInfoNew);
	return m_listDriveInfo.GetAt(pos);
}


void CDriveCheck::Reset()
{
	TRACE0("CDriveCheck::Reset()\n");
	POSITION pos = m_listDriveInfo.GetHeadPosition();
	while (pos != NULL)
		m_listDriveInfo.GetNext(pos).SetStatus(Status::Reset);
	m_dwDrives = 0;
}

CString CDriveCheck::CheckParentPath(const CString& strPath)
{
	int p = strPath.ReverseFind('\\');
	if (p > 0)
		return CheckDirPath(strPath.Left(p));
	return _T("bad path");
}


CString CDriveCheck::CheckPath(const CString& strPath, bool bDir)
{
	TRACE2("CheckPath %s d=%d\n", strPath, bDir);
	CDriveInfo driveInfo;
	CString strError = CheckRootPath(strPath, driveInfo);
	if (!strError.IsEmpty())
	{
		TRACE1("CheckPath root err=%s\n", strError);
		return strError;
	}

	CFileStatus fs;
	if (CFile::GetStatus(strPath, fs))
	{
		if (bDir && (fs.m_attribute & CFile::directory) == 0)
			return _T("File \"") + strPath + _T("\" is not a folder.");
		driveInfo.SetStatus(Status::Online);
	}
	else
	{
		strError = _T("Path \"") + strPath + _T("\" not found.");
		if (bDir)
			ResetPath(strPath);
	}
	TRACE1("CheckPath err=%s\n", strError);
	return strError;
}

CString CDriveCheck::CheckRootPath(const CString& strPath, CDriveInfo &driveInfoRes)
{
	TRACE1("CheckRootPath %s\n", strPath);
	CString strError = CheckNetPath(strPath, driveInfoRes);
	if (!strError.IsEmpty())
		return strError;

	return CheckDrive(strPath);
}

CString CDriveCheck::CheckRootPath(const CString& strPath)
{
	CDriveInfo driveInfo;
	return CheckRootPath(strPath, driveInfo);
}

CString CDriveCheck::CheckDrive(const CString& strPath) {
	TRACE1("CheckDrive %s\n", strPath);
	if (strPath.IsEmpty() || strPath[0] != '\\')
	{
		if (strPath.GetLength() >= 2 && strPath[1] == ':') { // check drive letter
			int nDrive = toupper(strPath[0]) - 'A';
			TRACE1(" dw=0x%x\n", m_dwDrives);
			DWORD dwDrive = 1 << nDrive;
			if ((dwDrive & m_dwDrives) == 0)
				m_dwDrives = GetLogicalDrives();
			if ((dwDrive & m_dwDrives) == 0) {
				return _T("Drive \"") + strPath.Left(2) + _T("\" is down.");
			}
		}
		return CString();	// drive found
	}
	return CString();		// network path can't be checked
}

CString CDriveCheck::CheckNetPath(const CString& strPath, CDriveInfo &driveInfoRes) {
	if (strPath.Left(2) == _T("\\\\"))
	{
		TRACE1("CheckNetPath %s\n", strPath);
		int iStart = 2;
		CString strSrv = strPath.Tokenize(_T("\\"), iStart);
		CDriveInfo &driveInfoSrv = GetNetDriveInfo(strSrv);
		driveInfoRes = driveInfoSrv;
		if (driveInfoRes.IsCurrent())
			return driveInfoRes.StatusMsg();

		DWORD dwType = 0;
		BOOL bNetAlive = IsNetworkAlive(&dwType);
		DWORD dwLE = GetLastError();
		if (dwLE == 0 && (!bNetAlive || (dwType & NETWORK_ALIVE_LAN) == 0))
		{
			driveInfoSrv.SetStatus(Status::NoNet);
			driveInfoRes = driveInfoSrv;
			return driveInfoRes.StatusMsg();	//return _T("network is not alive.");
		}

		TRACE1("CheckNetPath ping %s\n", strSrv);
		CPing ping;
		BOOL bRetH = ping.SetHostIP(CStringA(strSrv));
		BOOL bRetP = bRetH && ping.SendEcho();
//		if (bRetH && !bRetP)
//			bRetP = ping.SendEcho();	// retrying
		if (!bRetP)
		{
			if (driveInfoSrv.WakeLan())
				driveInfoSrv.SetStartTime();
			else
				TRACE0("WakeLan false\n");
			driveInfoSrv.SetStatus(Status::NoPing);
			driveInfoRes = driveInfoSrv;
			TRACE1("ping=%s\n", driveInfoRes.StatusMsg());
			return driveInfoRes.StatusMsg();
		}
		TRACE0("ping=ok\n");

		if (!driveInfoSrv.NetConn())
		{
			driveInfoSrv.SetStatus(Status::Connecting);
			driveInfoRes = driveInfoSrv;
			TRACE1("conn=%s\n", driveInfoRes.StatusMsg());
			return driveInfoRes.StatusMsg();
		}

		if (!driveInfoSrv.IsStarted())
		{
			driveInfoSrv.SetStatus(Status::Starting);	//  retry after 10 min
			driveInfoRes = driveInfoSrv;
			TRACE1("conn=%s\n", driveInfoRes.StatusMsg());
			return driveInfoRes.StatusMsg();
		}
		driveInfoSrv.SetStatus(Status::Running);
		driveInfoRes = driveInfoSrv;
		TRACE0("running\n");
	}
	return CString();	// ok: running or no net drive
}

void CDriveCheck::ResetPath(const CString strPath)
{
	if (strPath.GetLength() <= 2)
		return;
	if (strPath[1] == ':') { // check drive letter
		int nDrive = toupper(strPath[0]) - 'A';
		DWORD dwDrive = 1 << nDrive;
		m_dwDrives &= ~dwDrive;
		return;
	}
	if (strPath[0] == '\\' && strPath[1] == '\\')
	{
		int iStart = 2;
		CString strSrv = strPath.Tokenize(_T("\\"), iStart);
		CDriveInfo& driveInfoSrv = GetNetDriveInfo(strSrv);
		driveInfoSrv.SetStatus(Status::Reset);
	}
}


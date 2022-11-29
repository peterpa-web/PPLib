#include "stdafx.h"
#include "Ping.h"
#include "SensAPI.h"	// IsNetworkAlive 
#include "EventLog.h"

#include "DriveCheck.h"

#pragma comment( lib, "SensAPI" ) 

CStringW CDriveCheck::CDriveInfo::s_strUser;
CStringW CDriveCheck::CDriveInfo::s_strPasswd;
CStringW CDriveCheck::CDriveInfo::s_strShareName;

CDriveCheck::CDriveInfo::CDriveInfo()
{
	m_timeUpd = CTime::GetCurrentTime();
}

CDriveCheck::CDriveInfo::CDriveInfo(const CString& strName)
{
	m_strName = strName;
	m_strName.MakeLower();
	m_timeUpd = CTime::GetCurrentTime();
}

bool CDriveCheck::CDriveInfo::IsCurrent()
{
	if (IsForce())
		return false;
	return m_timeUpd > CTime::GetCurrentTime();
}

void CDriveCheck::CDriveInfo::SetStatus(enum Status stat)
{
	int nSecs = 60;
	if (stat == Status::NoPing)
		nSecs = 600;
	TRACE3("CDriveInfo::SetStatus %d %s %ds\n", stat, (LPCTSTR)m_strName, nSecs);
	if (stat == Status::Force && m_status < Status::Running)
		return;

	if (stat <= m_statusNext)
		m_strNetStatus.Empty();
	if (stat <= m_statusNext || CTime::GetCurrentTime() >= m_timeUpd)
	{
		if (stat == Status::Running)
		{
			TRACE0("SetStatus running\n");
			if (s_strShareName.Find(m_strName) >= 0)
			{
				if (NetConn())
				{
					TRACE0("SetStatus after NetConn\n");
					CTimeSpan d(nSecs);
					m_timeUpd = CTime::GetCurrentTime() + d;
					m_status = stat;
				}
				else
					TRACE1("SetStatus NetConn failed: %s\n", m_strNetStatus);
			}
			else
				m_strNetStatus.Empty();
		}
		else
		{
			TRACE2("SetStatus st=%d upd time (n=%d)\n", stat, m_statusNext);
			CTimeSpan d(nSecs);
			m_timeUpd = CTime::GetCurrentTime() + d;
			m_status = stat;
		}
		TRACE1(" stat=%s\n", (LPCTSTR)StatusMsg());
	}
	else
	{
		if (stat == Status::Online)
		{
			TRACE0("SetStatus online upd time\n");
			CTimeSpan d(nSecs);
			m_timeUpd = CTime::GetCurrentTime() + d;
			m_status = stat;
		}
		else
			TRACE2("SetStatus st=%d delayed (n=%d)\n", stat, m_statusNext);
	}
	m_statusNext = stat;
}

bool CDriveCheck::CDriveInfo::NetConn()
{
	if (m_strNetStatus == _T("ok"))
		return true;

	TRACE1("NetConn %s\n", s_strShareName);
	NETRESOURCE netrc;
	netrc.dwScope = RESOURCE_GLOBALNET;
	netrc.dwType = RESOURCETYPE_DISK;
	netrc.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;
	netrc.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	netrc.lpLocalName = NULL;
	netrc.lpRemoteName = (LPTSTR)(LPCTSTR)s_strShareName;
	netrc.lpProvider = NULL;

	DWORD dwErr = NO_ERROR;

	if (s_strUser.IsEmpty())
		dwErr = WNetAddConnection2(&netrc, NULL, NULL, 0);
	else
		dwErr = WNetAddConnection2(&netrc, s_strPasswd, s_strUser, 0);

	if (dwErr == NO_ERROR)
	{
		m_strNetStatus = _T("ok");
		return true;
	}
	if (dwErr == ERROR_BAD_NET_NAME ||
		dwErr == ERROR_BAD_NETPATH)
	{
		m_strNetStatus = _T("server ") + s_strShareName + _T(" not found");
		TRACE1("No Connection to %s\n", s_strShareName);
		return false;
	}
	if (dwErr == ERROR_SESSION_CREDENTIAL_CONFLICT)	// 1219
	{
		TRACE1("ERROR_SESSION_CREDENTIAL_CONFLICT %s\n", s_strShareName);
		dwErr = WNetCancelConnection2(netrc.lpRemoteName, 0, TRUE);
		m_strNetStatus = _T("server ") + s_strShareName + _T(" credential conflict");
		if (dwErr == NO_ERROR)
		{
			TRACE0("CancelConn done\n");
			return false;	// retry AddConn later
		}
		// else set status below
	}
	if (dwErr == ERROR_ALREADY_ASSIGNED)		// 85
	{
		m_strNetStatus = _T("server ") + s_strShareName + _T(" ERROR_ALREADY_ASSIGNED");
		TRACE1("ERROR_ALREADY_ASSIGNED %s\n", s_strShareName);
		return false;
	}
	if (dwErr == ERROR_IO_PENDING)	// 997
	{
		m_strNetStatus = _T("server ") + s_strShareName + _T(" ERROR_IO_PENDING");
		TRACE1("ERROR_IO_PENDING %s\n", s_strShareName);
		return false;
		//		Sleep( 2000 );
		//		dwErr = NO_ERROR;	// no retry
	}
	if (dwErr == ERROR_LOGON_FAILURE)	// 1326
	{
		m_strNetStatus = _T("server ") + s_strShareName + _T(" ERROR_LOGON_FAILURE");
		TRACE1("ERROR_LOGON_FAILURE %s\n", s_strShareName);
		return false;
	}
	m_strNetStatus = CEventLogException::GetLastErrorText(dwErr);
	return false;
}

CString CDriveCheck::CDriveInfo::StatusMsg()
{
	if (IsRunning())
		return _T("");
	if (HasNoNet())
		return _T("Network is not alive.");
	if (!m_strNetStatus.IsEmpty())
		return m_strNetStatus;
	if (HasNoPing())
//		return _T("Server \"") + m_strName + _T("\" gives no ping response.");
		return _T("Waiting for server \"") + m_strName + _T("\".");
	return _T("Server \"") + m_strName + _T("\" is not running.");
}


CDriveCheck::CDriveInfo& CDriveCheck::GetNetDriveInfo(CString strSrv)
{
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
	m_listDriveInfo.RemoveAll();
	m_dwDrives = 0;
}

CString CDriveCheck::CheckParentPath(const CString& strPath, bool bForce)
{
	int p = strPath.ReverseFind('\\');
	if (p > 0)
		return CheckDirPath(strPath.Left(p), bForce);
	return _T("bad path");
}


CString CDriveCheck::CheckPath(const CString& strPath, bool bForce, bool bDir)
{
	TRACE3("CheckPath %s f=%d d=%d\n", strPath, bForce, bDir);
	CDriveInfo driveInfo;
	if (bForce)
		driveInfo.SetStatus(Status::Force);
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
		if (!driveInfo.IsUnknown())
			driveInfo.SetStatus(Status::Online);
	}
	else
	{
		if (!driveInfo.IsUnknown())
		{
			if (driveInfo.IsOnline())
				strError = _T("Path \"") + strPath + _T("\" doesn't exist!");
			else
				strError = _T("(Path \"") + strPath + _T("\" temporary not available.)");
		}
		else
		{
			strError = _T("Path \"") + strPath + _T("\" not found.");
			if (bDir)
				RemoveDrive(strPath);
		}
		return strError;
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
		bool bForce = driveInfoRes.IsForce();
		CDriveInfo &driveInfoSrv = GetNetDriveInfo(strSrv);
		driveInfoRes = driveInfoSrv;
		if (!bForce) {
			if (driveInfoRes.IsCurrent())
				return driveInfoRes.StatusMsg();
		}

		DWORD dwType = 0;
		BOOL bNetAlive = IsNetworkAlive(&dwType);
		if (!bNetAlive || (dwType != NETWORK_ALIVE_LAN))
		{
			driveInfoSrv.SetStatus(Status::NoNet);
			driveInfoRes = driveInfoSrv;
			return driveInfoRes.StatusMsg();
			//return _T("network is not alive.");
		}

		TRACE1("CheckNetPath ping %s\n", strSrv);
		CPing ping;
		BOOL bRetH = ping.SetHostIP(CStringA(strSrv));
		BOOL bRetP = bRetH && ping.SendEcho();
		if (bRetH && !bRetP)
			bRetP = ping.SendEcho();
		if (bRetP)
		{
			driveInfoSrv.SetStatus(Status::Running);
			driveInfoRes = driveInfoSrv;
			TRACE0("ping=ok\n");
			return driveInfoRes.StatusMsg();
		}
		driveInfoSrv.SetStatus(Status::NoPing);	//  retry after 10 min
		driveInfoRes = driveInfoSrv;
		TRACE1("ping=%s\n", driveInfoRes.StatusMsg());
		return driveInfoRes.StatusMsg();
	}
	return CString();	// ok: no net drive
}

void CDriveCheck::RemoveDrive(const CString strPath)
{
	if (strPath.GetLength() > 2 && strPath[1] == ':') { // check drive letter
		int nDrive = toupper(strPath[0]) - 'A';
		DWORD dwDrive = 1 << nDrive;
		m_dwDrives &= ~dwDrive;
	}
}


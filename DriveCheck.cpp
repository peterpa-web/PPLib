#include "stdafx.h"
#include "Ping.h"
#include "SensAPI.h"	// IsNetworkAlive 

#include "DriveCheck.h"

#pragma comment( lib, "SensAPI" ) 

CDriveCheck::CDriveInfo::CDriveInfo()
{
//	m_timeActive = 
	m_timeUpd = CTime::GetCurrentTime();
}

CDriveCheck::CDriveInfo::CDriveInfo(const CString& strName)
{
	m_strName = strName;
	m_strName.MakeLower();
//	m_timeActive = 
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
		nSecs = 300;
	TRACE3("CDriveInfo::SetStatus %d %s %ds\n", stat, (LPCTSTR)m_strName, nSecs);
	if (stat == Status::Force && m_status < Status::Running)
		return;

	if (stat <= m_statusNext || CTime::GetCurrentTime() >= m_timeUpd)
	{
		m_status = stat;
		TRACE1(" %s\n", (LPCTSTR)StatusMsg());
	}
	else
		TRACE0(" delayed\n");
	m_statusNext = stat;
	CTimeSpan d(nSecs);
	m_timeUpd = CTime::GetCurrentTime() + d;
}

CString CDriveCheck::CDriveInfo::StatusMsg()
{
	if (IsRunning())
		return _T("");
	if (HasNoNet())
		return _T("network is not alive.");
	if (HasNoPing())
		return _T("Server \"") + m_strName + _T("\" gives no ping response.");
	//	driveInfoRes.WaitActive();
	if (IsRunning())
		return _T("");
	return _T("Server \"") + m_strName + _T("\" is not running.");
}

/*
void CDriveCheck::CDriveInfo::CheckActive()
{
	if (m_status != m_statusNext && m_timeActive <= CTime::GetCurrentTime() )
	{
		TRACE2("CDriveInfo::CheckActive %d -> %d\n", m_status, m_statusNext);
		m_status = m_statusNext;
	}
}

void CDriveCheck::CDriveInfo::WaitActive()
{
	if (m_status != m_statusNext && m_timeActive > CTime::GetCurrentTime() )
	{
		CTimeSpan ts(m_timeActive- CTime::GetCurrentTime());
		TRACE2("CDriveInfo::WaitActive %s for %ds\n", m_strName, ts.GetSeconds());
		TRACE2(" status %d -> %d\n", m_status, m_statusNext);
		Sleep(ts.GetSeconds() * 1000);
		m_status = m_statusNext;
	}
}
*/


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


CString CDriveCheck::CheckDirPath(const CString& strPath, bool bForce)
{
	TRACE1("CheckDirPath %s\n", strPath);
	CDriveInfo driveInfo;
	if (bForce)
		driveInfo.SetStatus(Status::Force);
	CString strError = CheckRootPath(strPath, driveInfo);
	if (!strError.IsEmpty())
	{
		TRACE1("CheckDirPath err=%s\n", strError);
		return strError;
	}

	CFileStatus fs;
	if (!CFile::GetStatus(strPath, fs) ||
		(fs.m_attribute & CFile::directory) == 0)
	{
		if (!driveInfo.IsUnknown())
		{
			if (driveInfo.IsOnline())
				strError = _T("Folder \"") + strPath + _T("\" doesn't exist!");
			else
				strError = _T("(Folder \"") + strPath + _T("\" temporary not available.)");
		}
		else
		{
			strError = _T("Folder \"") + strPath + _T("\" not found.");
			RemoveDrive(strPath);
		}
		return strError;
	}
	if (!driveInfo.IsUnknown())
		driveInfo.SetStatus(Status::Online);
	TRACE1("CheckDirPath err=%s\n", strError);
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
		ping.SetHostIP(CStringA(strSrv));
		if (ping.SendEcho())
		{
			driveInfoSrv.SetStatus(Status::Running);
			driveInfoRes = driveInfoSrv;
			return _T("");	// ok
		}
		driveInfoSrv.SetStatus(Status::NoPing);	//  retry after 5 min
		driveInfoRes = driveInfoSrv;
		return driveInfoRes.StatusMsg();
		//return _T("Server \"") + strSrv + _T("\" gives no ping response.");
	}
	return _T("");	// no net drive
}

void CDriveCheck::RemoveDrive(const CString strPath)
{
	if (strPath.GetLength() > 2 && strPath[1] == ':') { // check drive letter
		int nDrive = toupper(strPath[0]) - 'A';
		DWORD dwDrive = 1 << nDrive;
		m_dwDrives &= ~dwDrive;
	}
}


#pragma once
class CDriveCheck
{
public:
	enum class Status {
		Unknown = -2,
		Reset = -1,
		Offline = 0,
		NoNet,
		NoPing,
		Connecting,
		Starting,
		Running,
		Online
	};

	class CDriveInfo
	{
	public:
		CDriveInfo();
		CDriveInfo(const CString& strName);
		const CString& GetName() const { return m_strName; }
		void SetWakeLanData(const CStringA& strWakeNetAddr, unsigned int mac[6]);
		void SetConnData(const CStringW& strShareName, const CStringW& strUser, const CStringW& strPasswd) {
			if (strShareName.Find(m_strName) >= 0)
				m_strShareName = strShareName; m_strUser = strUser; m_strPasswd = strPasswd;
		}
		void SetStatus(enum class Status stat);
		bool IsCurrent() { return m_status >= Status::Offline && m_timeUpd > CTime::GetCurrentTime(); }
		bool IsUnknown() { return m_status == Status::Unknown; }
		bool HasNoNet() { return m_status == Status::NoNet; }
		bool HasNoPing() { return m_status == Status::NoPing; }
		bool IsStarting() { return m_status == Status::Starting; }
		bool IsConnecting() { return m_status == Status::Connecting; }
		bool IsRunning() { return m_status >= Status::Running; }
		bool IsOnline() { return m_status == Status::Online; }
		LONG GetValidSecs() { CTimeSpan ts(m_timeUpd - CTime::GetCurrentTime()); return (LONG)ts.GetTotalSeconds(); }
		bool WakeLan();
		void SetStartTime() {
			m_timeStart = CTime::GetCurrentTime();
		}
		bool IsStarted() { 
			CTimeSpan ts(CTime::GetCurrentTime() - m_timeStart);
			return ts.GetTotalSeconds() >= 600;	// 10min
		}
		bool NetConn();
		CString StatusMsg();

	protected:
		CString m_strName;
		CTime m_timeStart;
		CTime m_timeUpd;	// no re-check before this time; see GetValidSecs(), IsCurrent()
		CString m_strNetConnStatus;
		enum class Status m_status = Status::Unknown;

		CStringA m_strWakeNetAddr;
		unsigned int m_mac[6] = { 0,0,0,0,0,0 };

		CStringW m_strShareName;
		CStringW m_strUser;
		CStringW m_strPasswd;
	};

	void Reset();
	void ResetPath(const CString strPath);
	CString CheckParentPath(const CString& strPath); // returns error msg
	CString CheckPath(const CString& strPath, bool bDir = false); // returns error msg
	CString CheckDirPath(const CString& strPath) { return CheckPath(strPath, true); }
	CString CheckRootPath(const CString& strPath); // returns error msg
	CString CheckRootPath(const CString& strPath, CDriveInfo &driveInfoRes);
//	void InitDrives() { 
//		m_dwDrives = 0; m_listDriveInfo.RemoveAll(); 
//	}
	void SetWakeLanData(const CStringW& strShareName, const CStringA& strWakeNetAddr, unsigned int mac[6]) {
		GetNetDriveInfo(strShareName).SetWakeLanData(strWakeNetAddr, mac);
	}
	void SetConnData(const CStringW& strShareName, const CStringW& strUser, const CStringW& strPasswd) {
		GetNetDriveInfo(strShareName).SetConnData(strShareName, strUser, strPasswd);
	}

protected:
	CList<CDriveInfo> m_listDriveInfo;
	DWORD m_dwDrives = 0;

	CDriveInfo& GetNetDriveInfo(CString strSrv);
	CString CheckDrive(const CString& strPath);
	CString CheckNetPath(const CString& strPath, CDriveInfo &driveInfoRes);
};


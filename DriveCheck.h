#pragma once
class CDriveCheck
{
protected:
	enum class Status {
		Force = -2,
		Unknown = -1,
		Offline = 0,
		NoNet,
		NoPing,
		Running,
		Online
	};

	class CDriveInfo
	{
	public:
		CDriveInfo();
		CDriveInfo(const CString& strName);
		const CString& GetName() const { return m_strName; }
		bool IsCurrent();
		bool IsForce() { return m_status == Status::Force; }
		bool IsUnknown() { return m_status == Status::Unknown; }
	//	bool IsUnknown() { CheckActive(); return m_status == Status::Unknown; }
		//bool IsOffline() { return m_status == Offline; }
		bool HasNoNet() { return m_status == Status::NoNet; }
		bool HasNoPing() { return m_status == Status::NoPing; }
		bool IsRunning() { return m_status >= Status::Running; }
//		bool IsRunning() { CheckActive(); return m_status >= Status::Running; }
		bool IsOnline() { return m_status == Status::Online; }
//		bool IsOnline() { CheckActive(); return m_status == Status::Online; }
		void SetStatus(enum class Status stat, int nSecs = 60);
		LONG GetValidSecs() { CTimeSpan ts(m_timeUpd - CTime::GetCurrentTime()); return (LONG)ts.GetTotalSeconds(); }
	//	void WaitActive();
		CString StatusMsg();

	protected:
	//	void CheckActive();

		CString m_strName;
		CTime m_timeUpd;	// no re-check before this time; see IsCurrent()
	//	CTime m_timeActive;	// delayed active statusNext -> status
		enum class Status m_status = Status::Unknown;
	//	enum class Status m_statusNext = Status::Unknown;
	};

public:
	CString CheckParentPath(const CString& strPath, bool bForce = false); // returns error msg
	CString CheckDirPath(const CString& strPath, bool bForce = false); // returns error msg
	CString CheckRootPath(const CString& strPath); // returns error msg

protected:
	CList<CDriveInfo> m_listDriveInfo;
	CDriveInfo& GetNetDriveInfo(CString strSrv);

	CString CheckRootPath(const CString& strPath, CDriveInfo &driveInfoRes);
	CString CheckDrive(const CString& strPath);
	CString CheckNetPath(const CString& strPath, CDriveInfo &driveInfoRes);
};


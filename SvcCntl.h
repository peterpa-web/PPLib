// sniSvcCntl.h: interface of the CsniSvcCntl class
//
#pragma once

#include <winsvc.h>
#include "EventLog.h"

class CSvcCntl : public CObject
{
protected: 
	SC_HANDLE m_hSCM;
	SC_HANDLE m_hService;
	CString m_strServiceName;
	SERVICE_STATUS m_svcStat;
	bool m_bOpenSCM;

// Attributes
public:
	CSvcCntl(
		LPCTSTR lpServiceName = NULL, 
		DWORD dwDesiredAccess = SERVICE_ALL_ACCESS,
		DWORD dwDesiredSCAccess = SC_MANAGER_ALL_ACCESS,
		SC_HANDLE= NULL );
	SC_HANDLE GetServiceHandle() { return m_hService; }

// Operations
public:
	void CreateService( 
		LPCTSTR lpServiceName,		// pointer to service name 
		LPCTSTR lpDisplayName,		// pointer to display name 
		LPCTSTR lpBinaryPathName,	// pointer to name of binary file 
		DWORD dwStartType = SERVICE_DEMAND_START,	// when to start service 
		LPCTSTR lpServiceAccName = NULL,	// pointer to account name of service 
		LPCTSTR lpPassword = NULL, 			// pointer to password for service account 
		DWORD dwDesiredAccess = SERVICE_ALL_ACCESS,
		DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS,
		LPCTSTR lpDependencies = NULL,		// pointer to array of dependency names 
		LPCTSTR lpLoadOrderGroup = NULL,	// pointer to name of load ordering group 
		LPDWORD lpdwTagId = NULL,			// pointer to variable to get tag identifier 
		DWORD dwErrorControl = SERVICE_ERROR_NORMAL		// severity if service fails to start 
	);

	void QueryServiceStatus( 
		DWORD& dwCurrentState );

	void ControlService(
		DWORD dwControl = SERVICE_CONTROL_INTERROGATE );

	void StartService(
		CStringArray* paServiceArgs = NULL );

	void StopService();

	void StopServiceRecursive();

	void DeleteService();

	void EnumDependentServices( DWORD dwServiceState, CStringArray &astrSvcNames );

// Implementation
public:
	virtual ~CSvcCntl();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

};

// ------------------------------------

class CSvcCntlException : public CEventLogException
{
	static DWORD m_dwMsgBase;

protected:
	DWORD m_nLastError;
	CString m_strServiceName;

// Attributes
public:
	DECLARE_DYNAMIC( CSvcCntlException )
	CSvcCntlException( 
		LPCTSTR lpServiceName, 
		LPCTSTR pszMsg );

	static DWORD GetMsgBase() { return m_dwMsgBase; }
	static void SetMsgBase( DWORD dwMsgBase ) { m_dwMsgBase = dwMsgBase; }
	DWORD GetLastError() { return m_nLastError; }

// Operations
public:
	virtual BOOL GetErrorMessage( 
		LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext );

	virtual BOOL ReportEvent( 
		CEventLog* pEventLog,
		WORD wType = EVENTLOG_ERROR_TYPE,
		WORD wCategory = 0 );

// Implementation
public:
	virtual ~CSvcCntlException();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};



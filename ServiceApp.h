// ServiceApp.h : main header file for the CServiceApp class
//

#pragma once

#include "EventLog.h"
#include <winsvc.h>
#include <afxmt.h>
#include "registry.h"
#include "ConsApp.h"

#define SERVICE_CONTROL_USER 128

/////////////////////////////////////////////////////////////////////////////

// globals

// int main( int argc, char *argv[] );

/////////////////////////////////////////////////////////////////////////////

class CServiceApp : public CConsApp
{
	static CServiceApp* m_pApp;

	static BOOL WINAPI ControlHandlerStatic( DWORD dwCtrlType );
	static void WINAPI ServiceMainStatic( DWORD dwArgc, LPTSTR *lpszArgv );
	static void WINAPI ServiceCtrlStatic( DWORD dwCtrlCode );

//	friend int ::main( int argc, char *argv[] );

protected:
	static BOOL	m_bDebug;

	CString	m_strServiceName;
	CString	m_strDisplayName;
//	CEventLog*	m_pEventLog;
	SERVICE_STATUS	m_ss;
	SERVICE_STATUS_HANDLE	m_ssh;
	CEvent* m_pevServiceStop;
	CString m_strArg2;
	CString m_strArg3;
	CString m_strArg4;

	static CServiceApp* GetThis() { ASSERT( m_pApp ); return m_pApp; }

	virtual void Init();
	void InitEventLog(
			DWORD dwDefaultMsgBase = 0,
			DWORD dwRegMsgBase = 100,
			DWORD dwSvcCntlMsgBase = 200 );
	virtual int main( int argc, TCHAR *argv[] );
	void ServiceMain( DWORD dwArgc, LPTSTR *lpszArgv );
	virtual void ServiceStart( DWORD dwArgc, LPTSTR *lpszArgv );
	virtual void Startup( DWORD dwArgc, LPTSTR *lpszArgv ) = 0;
	virtual void Run() = 0;
	virtual void ServiceStop();
	virtual void ServiceCtrl( DWORD dwCtrlCode );
	void ReportStatusToSCMgr( DWORD dwCurrentState,
							  DWORD dwWin32ExitCode = NO_ERROR,
							  DWORD dwWaitHint = 3000 );
	virtual void CmdInstallService();
	virtual void CmdRemoveService();
	virtual void CmdDebugService( int argc, TCHAR *argv[] );
	CString GetDefaultKey( int nLevel = -1 );
	void CreateDefaultKey();
	virtual void CreateDefaultParms( CRegistry& reg );


public:
	CServiceApp( LPCTSTR pszServiceName, LPCTSTR pszDisplayName );
	~CServiceApp();

	static BOOL IsStopping() 
	{ 
		ASSERT( m_pApp ); 
		return m_pApp->m_ss.dwCurrentState == SERVICE_STOP_PENDING;
	}

	static void WriteError( CException *pe );

};


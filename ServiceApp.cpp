// ServiceApp.cpp : Defines the class CServiceApp. 
//
// Copyright 2001 Wincor Nixdorf Hamburg
//
// All rights reserved.
//
// author: Peter Pagel

#include "stdafx.h"
#include "ServiceApp.h"
#include "SvcCntl.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CServiceApp

CServiceApp* CServiceApp::m_pApp = NULL;		// static init
BOOL CServiceApp::m_bDebug = FALSE;

CServiceApp::CServiceApp( LPCTSTR pszServiceName, LPCTSTR pszDisplayName )
{
	ASSERT( m_pApp == NULL );
	m_pApp = this;

	ASSERT( pszServiceName );
	ASSERT( pszDisplayName );
	m_strServiceName = pszServiceName;
	m_strDisplayName = pszDisplayName;
//	m_pEventLog = NULL;
	m_pevServiceStop = NULL;
}

CServiceApp::~CServiceApp()
{
	m_pApp = NULL;
	delete m_pevServiceStop;
//	delete m_pEventLog;
}

void CServiceApp::Init()
{
//	AfxSetResourceHandle( GetModuleHandle( NULL ) );
	CConsApp::Init();

//	m_pEventLog = new CEventLog();
//	ASSERT( m_pEventLog );
	InitEventLog();
	m_pevServiceStop = new CEvent( FALSE, TRUE );
}

void CServiceApp::InitEventLog(
			DWORD dwDefaultMsgBase,			// = 0
			DWORD dwRegMsgBase,				// = 100
			DWORD dwSvcCntlMsgBase )		// = 200
{
//	ASSERT( m_pEventLog );
	CEventLog::Init(m_strServiceName);
//	m_pEventLog->SetMsgBase( dwDefaultMsgBase );

	CRegException::SetMsgBase( dwRegMsgBase );
	CSvcCntlException::SetMsgBase( dwSvcCntlMsgBase );
}

//
//  FUNCTION: virtual main
//
//  PURPOSE: entrypoint for service
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    main() either performs the command line task, or
//    call StartServiceCtrlDispatcher to register the
//    main service thread.  When this call returns,
//    the service has stopped, so exit.
//
int CServiceApp::main( int argc, TCHAR *argv[] )
{
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ NULL, (LPSERVICE_MAIN_FUNCTION)ServiceMainStatic },
		{ NULL, NULL }
	};
	dispatchTable[0].lpServiceName = (LPTSTR)(LPCTSTR)m_strServiceName;

	if ( (argc > 1) &&
		 ((*argv[1] == '-') || (*argv[1] == '/')) )
	{
		if ( argc > 2 )
			m_strArg2 = argv[2];

		if ( argc > 3 )
			m_strArg3 = argv[3];

		if ( argc > 4 )
			m_strArg4 = argv[4];

		if ( _tcsicmp( _T("install"), argv[1]+1 ) == 0 )
		{
			CmdInstallService();
			return 0;
		}
		else if ( _tcsicmp( _T("remove"), argv[1]+1 ) == 0 )
		{
			CmdRemoveService();
			return 0;
		}
		else if ( _tcsicmp( _T("debug"), argv[1]+1 ) == 0 )
		{
			CmdDebugService( argc, argv );
			return 0;
		}
	}
	// if it doesn't match any of the above parameters
	// the service control manager may be starting the service
	// so we must call StartServiceCtrlDispatcher
	_tprintf( _T("%s -install [{MAN|AUTO} [{<domain>|.}\\<user> [<password>]]] to install the service\n"), argv[0] );
	_tprintf( _T("%s -remove           to remove the service\n"), argv[0] );
	_tprintf( _T("%s -debug <params>   to run as a console app for debugging\n"), argv[0] );
	_tprintf( _T("\nStartServiceCtrlDispatcher being called.\n") );
	_tprintf( _T("This may take several seconds.  Please wait.\n") );

	if ( !StartServiceCtrlDispatcher( dispatchTable ) )
		throw new CSvcCntlException( m_strDisplayName,
			_T("CServiceApp::main() - StartServiceCtrlDispatcher()") );
	return 0;
}

//
//  FUNCTION: ServiceMainStatic
//
//  PURPOSE: To perform actual initialization of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine performs the service initialization and then calls
//    the user defined ServiceStart() routine to perform majority
//    of the work.
//
void WINAPI CServiceApp::ServiceMainStatic( DWORD dwArgc, LPTSTR *lpszArgv )
{
	ASSERT( m_pApp != NULL );
	try
	{
		m_pApp->ServiceMain( dwArgc, lpszArgv );
	}
	catch ( CException *pe )
	{
		m_pApp->WriteError( pe );
		CEventLog::GetInstance().Write( pe );
		pe->Delete();
	}
}

//
//  FUNCTION: ServiceCtrlStatic
//
//  PURPOSE: This function is called by the SCM whenever
//           ControlService() is called on this service.
//
//  PARAMETERS:
//    dwCtrlCode - type of control requested
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID WINAPI CServiceApp::ServiceCtrlStatic( DWORD dwCtrlCode )
{
	ASSERT( m_pApp != NULL );
	try
	{
		m_pApp->ServiceCtrl( dwCtrlCode );
	}
	catch ( CException *pe )
	{
		m_pApp->WriteError( pe );
		CEventLog::GetInstance().Write( pe );
		pe->Delete();
	}
}


///////////////////////////////////////////////////////////////////
//
//  The following code handles service installation and removal
//


//
//  FUNCTION: CmdInstallService()
//
//  PURPOSE: Installs the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CServiceApp::CmdInstallService()
{
	m_bDebug = TRUE;
    CString strPath = GetModuleFileName();
	if ( strPath.IsEmpty() )
    {
        throw new CSvcCntlException( m_strDisplayName, 
						_T("CServiceApp::CmdInstallService() - GetModuleFileName()") );
    }

	DWORD dwStartType = SERVICE_DEMAND_START;
	if ( m_strArg2.CompareNoCase( _T("Auto") ) == 0 )
		dwStartType = SERVICE_AUTO_START;

	LPCTSTR lpServiceStartName = NULL;
	if ( !m_strArg3.IsEmpty() )
		lpServiceStartName = m_strArg3;

	LPCTSTR lpDepend = _T("lanmanserver\0");

	CSvcCntl sc;
	sc.CreateService( 
		m_strServiceName,		// pointer to service name 
		m_strDisplayName,		// pointer to display name 
		strPath,				// pointer to name of binary file
		dwStartType,
		lpServiceStartName,		// user
		m_strArg4,				// password
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		lpDepend				// pointer to array of dependency names
	);

	CreateDefaultKey();

    _tprintf(TEXT("%s installed.\n"), (LPCTSTR)m_strDisplayName );
}



//
//  FUNCTION: CmdRemoveService()
//
//  PURPOSE: Stops and removes the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CServiceApp::CmdRemoveService()
{
	m_bDebug = TRUE;
	CSvcCntl sc( m_strServiceName );
	sc.DeleteService();

    _tprintf(TEXT("%s removed.\n"), (LPCTSTR)m_strDisplayName );
}




///////////////////////////////////////////////////////////////////
//
//  The following code is for running the service as a console app
//


//
//  FUNCTION: CmdDebugService(int argc, char ** argv)
//
//  PURPOSE: Runs the service as a console application
//
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CServiceApp::CmdDebugService( int argc, TCHAR *argv[] )
{
	m_bDebug = TRUE;
	m_ssh = (SERVICE_STATUS_HANDLE)1;	// dummy for debug

    _tprintf(TEXT("Debugging %s.\n"), (LPCTSTR)m_strDisplayName );

    SetConsoleCtrlHandler( ControlHandlerStatic, TRUE );

    ServiceStart( argc, argv );
}


//
//  FUNCTION: ControlHandlerStatic ( DWORD dwCtrlType )
//
//  PURPOSE: Handled console control events
//
//  PARAMETERS:
//    dwCtrlType - type of control event
//
//  RETURN VALUE:
//    True - handled
//    False - unhandled
//
//  COMMENTS:
//
BOOL WINAPI CServiceApp::ControlHandlerStatic( DWORD dwCtrlType )
{
    switch( dwCtrlType )
    {
        case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
        case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
			try
			{
				_tprintf(TEXT("Stopping %s.\n"), (LPCTSTR)(m_pApp->m_strDisplayName) );
				m_pApp->ReportStatusToSCMgr( SERVICE_STOP_PENDING, NO_ERROR, 0 );
				m_pApp->ServiceStop();
			}
			catch ( CException *pe )
			{
				m_pApp->WriteError( pe );
				CEventLog::GetInstance().Write( pe );
				pe->Delete();
			}
			return TRUE;
    }
    return FALSE;
}


//
//  FUNCTION: ServiceMain
//
//  PURPOSE: To perform actual initialization of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine performs the service initialization and then calls
//    the ServiceStart() routine to perform majority of the work.
//
void CServiceApp::ServiceMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // register our service control handler:
    //
    m_ssh = RegisterServiceCtrlHandler( (LPCTSTR)m_strServiceName, 
										ServiceCtrlStatic);
	if ( m_ssh == NULL )
        throw new CSvcCntlException( m_strDisplayName, 
						_T("CServiceApp::ServiceMain() - RegisterServiceCtrlHandler()") );

	ServiceStart( dwArgc, lpszArgv );
}

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: To perform actual initialization of the service
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    This routine calls the service initialization routine Startup()
//    and then calls the Run() routine to perform majority of the work.
//
void CServiceApp::ServiceStart( DWORD dwArgc, LPTSTR *lpszArgv )
{
	DWORD dwErr = 0;

	// SERVICE_STATUS members that don't change in example
	//
	m_ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_ss.dwServiceSpecificExitCode = 0;

	// report the status to the service control manager.
	//
	ReportStatusToSCMgr( SERVICE_START_PENDING );

	try
	{
		Startup( dwArgc, lpszArgv );
		ReportStatusToSCMgr( SERVICE_RUNNING, NO_ERROR, 0);
		Run();
	}
	catch ( CException *pe )
	{
		WriteError( pe );
		CEventLog::GetInstance().Write( pe );
		pe->Delete();
		dwErr = ERROR_INVALID_FUNCTION;
	}

    // report the stopped status to the service control manager.
    //
    ReportStatusToSCMgr( SERVICE_STOPPED, dwErr, 0 );
}

//
//  FUNCTION: ServiceCtrl
//
//  PURPOSE: This function is called by the SCM whenever
//           ControlService() is called on this service.
//
//  PARAMETERS:
//    dwCtrlCode - type of control requested
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CServiceApp::ServiceCtrl( DWORD dwCtrlCode )
{
    // Handle the requested control code.
    //
    switch ( dwCtrlCode )
    {
        // Stop the service.
        //
        case SERVICE_CONTROL_STOP:
			ReportStatusToSCMgr( SERVICE_STOP_PENDING, NO_ERROR, 0 );
            ServiceStop();
            break;

        // Update the service status.
        //
        case SERVICE_CONTROL_INTERROGATE:
			ReportStatusToSCMgr( m_ss.dwCurrentState, NO_ERROR, 0 );
            break;

        // invalid control code
        //
        default:
            break;

    }
}

//
//  FUNCTION: ReportStatusToSCMgr()
//
//  PURPOSE: Sets the current status of the service and
//           reports it to the Service Control Manager
//
//  PARAMETERS:
//    dwCurrentState - the state of the service
//    dwWin32ExitCode - error code to report
//    dwWaitHint - worst case estimate to next checkpoint
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
void CServiceApp::ReportStatusToSCMgr(
						DWORD dwCurrentState,
                        DWORD dwWin32ExitCode,	// = NO_ERROR
                        DWORD dwWaitHint)		// = 3000
{
    static DWORD dwCheckPoint = 1;

    if (dwCurrentState == SERVICE_START_PENDING)
        m_ss.dwControlsAccepted = 0;
    else
        m_ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    m_ss.dwCurrentState = dwCurrentState;
    m_ss.dwWin32ExitCode = dwWin32ExitCode;
    m_ss.dwWaitHint = dwWaitHint;

    if ( ( dwCurrentState == SERVICE_RUNNING ) ||
         ( dwCurrentState == SERVICE_STOPPED ) )
	{
        m_ss.dwCheckPoint = 0;
		dwCheckPoint = 1;
	}
    else
        m_ss.dwCheckPoint = dwCheckPoint++;

	if ( m_bDebug )
	{
		TRACE0( "ReportStatusToSCMgr: " );
		_tprintf( TEXT("ReportStatusToSCMgr: ") );
		switch ( dwCurrentState )
		{
			case SERVICE_STOPPED:
				TRACE1( "STOPPED (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("STOPPED (%d)\n"),  m_ss.dwCheckPoint );
				break;
			case SERVICE_START_PENDING:
				TRACE1( "START_PENDING (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("START_PENDING (%d)\n"),  m_ss.dwCheckPoint );
				break;
			case SERVICE_STOP_PENDING:
				TRACE1( "STOP_PENDING (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("STOP_PENDING (%d)\n"),  m_ss.dwCheckPoint );
				break;
			case SERVICE_RUNNING:
				TRACE1( "RUNNING (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("RUNNING (%d)\n"),  m_ss.dwCheckPoint );
				break;
			case SERVICE_CONTINUE_PENDING:
				TRACE1( "CONTINUE_PENDING (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("CONTINUE_PENDING (%d)\n"),  m_ss.dwCheckPoint );
				break;
			case SERVICE_PAUSE_PENDING:
				TRACE1( "PAUSE_PENDING (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("PAUSE_PENDING (%d)\n"),  m_ss.dwCheckPoint );
				break;
			case SERVICE_PAUSED:
				TRACE1( "PAUSED (%d)\n", m_ss.dwCheckPoint );
				_tprintf( TEXT("PAUSED (%d)\n"),  m_ss.dwCheckPoint );
				break;
			default:
				TRACE( " (%d)\n", dwCurrentState, m_ss.dwCheckPoint );
				_tprintf( TEXT("%d (%d)\n"), dwCurrentState, m_ss.dwCheckPoint );
		}
	}
	else
	{
		TRACE( "ReportStatusToSCMgr: %d (%d)\n", dwCurrentState, m_ss.dwCheckPoint );
        // Report the status of the service to the service control manager.
        //
        if ( !SetServiceStatus( m_ssh, &m_ss ) ) 
		{
			CString strState;
			strState.Format( _T("CServiceApp::ReportStatusToSCMgr() - SetServiceStatus(%d) [%d]"), dwCurrentState, m_ss.dwCheckPoint );
			throw new CSvcCntlException( m_strDisplayName, strState );
        }
    }
}

//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
void CServiceApp::ServiceStop()
{
	ASSERT( m_pevServiceStop );
    m_pevServiceStop->SetEvent();
}

//CString CServiceApp::GetModuleFileName() 
//{
//    TCHAR szPath[MAX_PATH];

//    if ( ::GetModuleFileName( NULL, szPath, MAX_PATH ) > 0 )
//	{
//		return szPath;
//	}
//	return _T("");
//}

//CString CServiceApp::GetDefaultFileName( LPCTSTR pszExt ) 
//{
//	CString strName = GetModuleFileName();
//	int nPosExt = strName.ReverseFind( '.' );
//	if ( nPosExt > 0 )
//	{
//		return strName.Left( nPosExt+1 ) + pszExt;
//	}
//	return _T("");
//}

//CString CServiceApp::GetDefaultPath() 
//{
//	CString strName = GetModuleFileName();
//	int nPosDelim = strName.ReverseFind( '\\' );
//	if ( nPosDelim > 0 )
//	{
//		return strName.Left( nPosDelim+1 );
//	}
//	return _T("");
//}

void CServiceApp::WriteError( CException *pe )
{
	if ( !m_bDebug )
		return;

	CConsApp::WriteError( pe );

//	const UINT nMaxError = 256;
//	_TCHAR szError[nMaxError];

//	pe->GetErrorMessage( szError, nMaxError );
//	_ftprintf( stderr, _T("%s\n"), szError );
}

CString CServiceApp::GetDefaultKey( int nLevel )	// = -1 
{
	LPCTSTR pszBase = _T("SYSTEM\\CurrentControlSet\\Services");
	LPCTSTR pszParms = _T("Parameters");

	switch ( nLevel )
	{
	case 0:
		return pszBase;

	case 1:
		return CString( pszBase ) + _T("\\") + m_strServiceName;

	case 2:
		return pszParms;

	default:
		break;
	}
	return CString( pszBase ) + _T("\\") + m_strServiceName 
			+ CString( _T("\\") ) + pszParms;
}

void CServiceApp::CreateDefaultKey()
{
	CRegistry reg;
	CString strSubkey = GetDefaultKey( 0 );

	reg.OpenSubKey( HKEY_LOCAL_MACHINE,
		GetDefaultKey( 1 ),
		KEY_CREATE_SUB_KEY );
	reg.CreateSubKey( GetDefaultKey( 2 ), KEY_ALL_ACCESS );
	CreateDefaultParms( reg );
	reg.CloseKey();
}

void CServiceApp::CreateDefaultParms( CRegistry& reg )
{
}

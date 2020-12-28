// sniSvcCntl.cpp: implementation of the CsniSvcCntl class
//

#include "stdafx.h"
#include "SvcCntl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CsniSvcCntl 

CSvcCntl::CSvcCntl(
		LPCTSTR lpServiceName,		// = NULL 
		DWORD dwDesiredAccess,		// = SERVICE_ALL_ACCESS
		DWORD dwDesiredSCAccess,	// = SC_MANAGER_ALL_ACCESS
		SC_HANDLE hSCM )			// = NULL
{
	m_hSCM = hSCM;
	m_hService = NULL;
	m_bOpenSCM = FALSE;

	if ( m_hSCM == NULL )
	{
		m_bOpenSCM = TRUE;
		m_hSCM = OpenSCManager( NULL, NULL, dwDesiredSCAccess );
		if ( m_hSCM == NULL )
			throw new CSvcCntlException( lpServiceName, _T("OpenSCManager") );
	}

	if ( lpServiceName != NULL )
	{
		m_strServiceName = lpServiceName;

		m_hService = ::OpenService( m_hSCM, lpServiceName, dwDesiredAccess);
		if ( m_hService == NULL )
			throw new CSvcCntlException( lpServiceName, _T("OpenService") );
	}
}

CSvcCntl::~CSvcCntl()
{
	if ( m_hService )
		CloseServiceHandle( m_hService );
	if ( m_bOpenSCM && m_hSCM != NULL )
		CloseServiceHandle( m_hSCM );
}

/////////////////////////////////////////////////////////////////////////////
// CsniSvcCntl diagnostics

#ifdef _DEBUG
void CSvcCntl::AssertValid() const
{
	CObject::AssertValid();
}

void CSvcCntl::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);

	dc << "m_hSCM = "								<< (void*)m_hSCM << "\n";
	dc << "m_hService = "							<< (void*)m_hService << "\n";
	dc << "m_strServiceName = "						<< m_strServiceName << "\n";
	dc << "m_svcStat.dwServiceType = "				<< m_svcStat.dwServiceType << "\n";
	dc << "m_svcStat.dwCurrentState = "				<< m_svcStat.dwCurrentState << "\n";
	dc << "m_svcStat.dwControlsAccepted = "			<< m_svcStat.dwControlsAccepted << "\n";
	dc << "m_svcStat.dwWin32ExitCode = "			<< m_svcStat.dwWin32ExitCode << "\n";
	dc << "m_svcStat.dwServiceSpecificExitCode = "	<< m_svcStat.dwServiceSpecificExitCode << "\n";
	dc << "m_svcStat.dwCheckPoint = "				<< m_svcStat.dwCheckPoint << "\n";
	dc << "m_svcStat.dwWaitHint = "					<< m_svcStat.dwWaitHint << "\n";
	dc << "m_bOpenSCM = "							<< m_bOpenSCM << "\n";
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CsniSvcCntl operations

void CSvcCntl::CreateService( 
    LPCTSTR lpServiceName,		// pointer to service name 
    LPCTSTR lpDisplayName,		// pointer to display name 
    LPCTSTR lpBinaryPathName,	// pointer to name of binary file 
    DWORD dwStartType,			// = SERVICE_DEMAND_START - when to start service 
    LPCTSTR lpServiceAccName,	// = NULL - pointer to account name of service 
    LPCTSTR lpPassword, 		// = NULL - pointer to password for service account 
    DWORD dwDesiredAccess,		// = SERVICE_ALL_ACCESS
    DWORD dwServiceType,		// = SERVICE_WIN32_OWN_PROCESS 
    LPCTSTR lpDependencies,		// = NULL - pointer to array of dependency names 
    LPCTSTR lpLoadOrderGroup,	// = NULL - pointer to name of load ordering group 
    LPDWORD lpdwTagId,			// = NULL - pointer to variable to get tag identifier 
    DWORD dwErrorControl		// = SERVICE_ERROR_NORMAL - severity if service fails to start 
)
{
	if ( m_hSCM == NULL )
		throw new CSvcCntlException( lpServiceName, _T("CreateService: ? SCManager") );
	if ( m_hService != NULL )
		throw new CSvcCntlException( m_strServiceName, _T("CreateService: ? exists") );

	m_hService = ::CreateService(
            m_hSCM,	
            lpServiceName,
            lpDisplayName,
            dwDesiredAccess,
            dwServiceType,	
            dwStartType,
            dwErrorControl,	
            lpBinaryPathName, 
            lpLoadOrderGroup,
            lpdwTagId, 
            lpDependencies,	
            lpServiceAccName, 
            lpPassword ); 
	if ( m_hService == NULL )
		throw new CSvcCntlException( lpServiceName, _T("CreateService failed") );

	m_strServiceName = lpServiceName;
}

void CSvcCntl::QueryServiceStatus( 
		DWORD& dwCurrentState )
{
	if ( m_hSCM == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("QueryServiceStatus: ? SCManager") );
	if ( m_hService == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("QueryServiceStatus: ? Service") );

	if ( ! ::QueryServiceStatus( m_hService, &m_svcStat ) )
		throw new CSvcCntlException( m_strServiceName, _T("QueryServiceStatus Cmd") );

	dwCurrentState = m_svcStat.dwCurrentState;
}

void CSvcCntl::ControlService(
		DWORD dwControl /* = SERVICE_CONTROL_INTERROGATE */ )
{
	if ( m_hSCM == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("ControlService: ? SCManager") );
	if ( m_hService == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("ControlService: ? Service") );

	if ( ! ::ControlService( m_hService, dwControl, &m_svcStat ) )
		throw new CSvcCntlException( m_strServiceName, _T("ControlService Cmd") );
}

void CSvcCntl::StartService(
		CStringArray* paServiceArgs /* = NULL */ )
{
	const int nMaxStrings = 20;
	LPCTSTR alpString[ nMaxStrings ];
	DWORD dwNumServiceArgs = 0;
	DWORD n;

	if ( m_hSCM == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("StartService: ? SCManager") );
	if ( m_hService == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("StartService: ? Service") );

	if ( paServiceArgs )
	{
		paServiceArgs->AssertValid();
		dwNumServiceArgs = (int)paServiceArgs->GetSize();
		ASSERT( dwNumServiceArgs < nMaxStrings );
		for ( n = 0; n < dwNumServiceArgs; n++ )
		{
			alpString[ n ] = paServiceArgs->GetAt( n );
		}
	}

	if ( ::QueryServiceStatus( m_hService, &m_svcStat ) &&
		 m_svcStat.dwCurrentState == SERVICE_STOPPED )
	{
		BOOL fStarted = FALSE;

		for ( n = 0; n < 60; n++ )
		{
			fStarted = ::StartService( m_hService, dwNumServiceArgs, alpString );
			if ( fStarted )
				break;

			if ( GetLastError() != ERROR_SERVICE_DATABASE_LOCKED )
				throw new CSvcCntlException( m_strServiceName, _T("StartService Cmd") );

			Sleep( 5000 );			// wait and retry
		}
		if ( ! fStarted )
			throw new CSvcCntlException( m_strServiceName, _T("StartService Locked") );
	}

	for ( n = 0; n < 60; n++ )
	{
		if ( ::QueryServiceStatus( m_hService, &m_svcStat ) )
		{
			if ( m_svcStat.dwCurrentState == SERVICE_RUNNING )
				return;

			if ( m_svcStat.dwCurrentState == SERVICE_STOPPED ||
				 m_svcStat.dwCurrentState == SERVICE_START_PENDING )
			{
				DWORD dwMilliseconds = m_svcStat.dwWaitHint;
				if ( dwMilliseconds <  1000 ) dwMilliseconds =  1000;
				if ( dwMilliseconds > 10000 ) dwMilliseconds = 10000;
				Sleep ( dwMilliseconds );
				continue;
			} 
			CString strMsg;
			strMsg.Format( _T("StartService: Status=0x%x"), m_svcStat.dwCurrentState );
			throw new CSvcCntlException( m_strServiceName, strMsg );
		}
		throw new CSvcCntlException( m_strServiceName, _T("StartService ?Status") );
	}
	throw new CSvcCntlException( m_strServiceName, _T("StartService Timeout") );
}

void CSvcCntl::StopService()
{
	if ( m_hSCM == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("StopService: ? SCManager") );
	if ( m_hService == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("StopService: ? Service") );

	if ( ::QueryServiceStatus( m_hService, &m_svcStat ) &&
		 m_svcStat.dwCurrentState == SERVICE_RUNNING )
	{
		if ( ! ::ControlService( m_hService, SERVICE_CONTROL_STOP, &m_svcStat ) )
			throw new CSvcCntlException( m_strServiceName, _T("StopService Cmd") );
	}

	for ( int n = 0; n < 60; n++ )
	{
		if ( ::QueryServiceStatus( m_hService, &m_svcStat ) )
		{
			if ( m_svcStat.dwCurrentState == SERVICE_STOPPED )
				return;

			if ( m_svcStat.dwCurrentState == SERVICE_RUNNING ||
				 m_svcStat.dwCurrentState == SERVICE_STOP_PENDING )
			{
				DWORD dwMilliseconds = m_svcStat.dwWaitHint;
				if ( dwMilliseconds <  1000 ) dwMilliseconds =  1000;
				if ( dwMilliseconds > 10000 ) dwMilliseconds = 10000;
				Sleep ( dwMilliseconds );
				continue;
			}
			CString strMsg;
			strMsg.Format( _T("StopService: Status=0x%x"), m_svcStat.dwCurrentState );
			throw new CSvcCntlException( m_strServiceName, strMsg );
		}
		throw new CSvcCntlException( m_strServiceName, _T("StopService: ?Status") );
	}
	throw new CSvcCntlException( m_strServiceName, _T("StopService Timeout") );
}

void CSvcCntl::StopServiceRecursive()
{
	CStringArray astrSvcNames;
	EnumDependentServices( SERVICE_ACTIVE, astrSvcNames );

	for ( int n = 0; n < astrSvcNames.GetSize(); n++ )
	{
		TRACE1( "Dep. %s\n", astrSvcNames[ n ] );
		CSvcCntl svc( astrSvcNames[ n ], SERVICE_ALL_ACCESS, 0, m_hSCM );
		svc.StopServiceRecursive();
	}
	StopService();
}

void CSvcCntl::DeleteService()
{
	StopService();
	if ( !::DeleteService( m_hService ) )
		throw new CSvcCntlException( m_strServiceName, _T("DeleteService") );

	CloseServiceHandle( m_hService );
	m_hService = NULL;
}

void CSvcCntl::EnumDependentServices( DWORD dwServiceState, CStringArray &astrSvcNames )
{
	if ( m_hService == NULL )
		throw new CSvcCntlException( m_strServiceName, _T("EnumDependentServices: ? Service") );

	DWORD dwBytesNeeded = 0;
	DWORD dwServicesReturned = 0;
	ENUM_SERVICE_STATUS *pEnumSvcStatus = NULL;
	DWORD dwBufSize = 0;

	if ( ::EnumDependentServices( m_hService, dwServiceState, pEnumSvcStatus, dwBufSize, 
									&dwBytesNeeded, &dwServicesReturned ) )
		return;		// no dependencies

	if ( GetLastError() != ERROR_MORE_DATA )
		throw new CSvcCntlException( m_strServiceName, _T("EnumDependentServices GetSize") );

	pEnumSvcStatus = (ENUM_SERVICE_STATUS *) new BYTE[ dwBytesNeeded ];
	dwBufSize = dwBytesNeeded;

	if ( ! ::EnumDependentServices( m_hService, dwServiceState, pEnumSvcStatus, dwBufSize, 
									&dwBytesNeeded, &dwServicesReturned ) )
	{
		delete [] (BYTE*)pEnumSvcStatus;
		throw new CSvcCntlException( m_strServiceName, _T("EnumDependentServices Exec") );
	}

	TRACE1( "EnumDependentServices n=%d\n", dwServicesReturned );
	astrSvcNames.SetSize( dwServicesReturned );
	for ( DWORD n = 0; n < dwServicesReturned; n++ )
	{
		astrSvcNames[ n ] = CString( pEnumSvcStatus[ n ].lpServiceName );
	}
	delete [] (BYTE*)pEnumSvcStatus;
}

/////////////////////////////////////////////////////////////////////////////
// CsniSvcCntlException

DWORD CSvcCntlException::m_dwMsgBase = 0;	// static init

IMPLEMENT_DYNAMIC(CSvcCntlException, CEventLogException)

CSvcCntlException::CSvcCntlException(
	LPCTSTR lpServiceName,
	LPCTSTR pszMsg
	) : CEventLogException( m_dwMsgBase, pszMsg )
{
	m_nLastError = ::GetLastError();
	m_strServiceName = lpServiceName;

	TRACE3( "CSvcCntlException: %s %s Err=%d\n", 
		lpServiceName, pszMsg, m_nLastError );
}

CSvcCntlException::~CSvcCntlException()
{
}


/////////////////////////////////////////////////////////////////////////////
// CsniSvcCntlException diagnostics

#ifdef _DEBUG
void CSvcCntlException::AssertValid() const
{
	CEventLogException::AssertValid();
}

void CSvcCntlException::Dump(CDumpContext& dc) const
{
	CEventLogException::Dump(dc);
	dc << "m_strServiceName = " << m_strServiceName << "\n";
	dc << "m_nLastError = "		<< m_nLastError << "\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CsniSvcCntlException commands

BOOL CSvcCntlException::GetErrorMessage( 
		LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext )
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	if (nMaxError == 0 || lpszError == NULL)
		return FALSE;

	CString strMsg( "CSvcCntlException: " );
	strMsg += m_strServiceName + _T(" ");
	strMsg += m_strMsg;
	if ( m_nLastError != 0 )
		strMsg += _T(":\n") + GetLastErrorText( m_nLastError );

	lstrcpyn( lpszError, strMsg, nMaxError );
	return TRUE;
}

BOOL CSvcCntlException::ReportEvent( 
		CEventLog* pEventLog,
		WORD wType,		/* = EVENTLOG_ERROR_TYPE */
		WORD wCategory	/* = 0 */ )
{
	CString strMsg( "CSvcCntlException: " );
	strMsg += m_strMsg;

	CStringArray astrMsg;
	astrMsg.Add( strMsg );
	astrMsg.Add( m_strServiceName );
	astrMsg.Add( GetLastErrorText( m_nLastError ) );

	return ( pEventLog->Write( 
		wType,				// event type to log
		wCategory,			// event category
		m_dwMsg,			// event identifier 
		&astrMsg ) );		// strings to merge with message 
}


// EventLog.cpp: implementation of the CEventLog class
//
// Copyright 2006 Peter Pagel
//
// All rights reserved.
// see also Platform SDK: Debugging and Error Handling
//						  Message Compiler / Message Text Files
//

#include "stdafx.h"
#include "registry.h"
#include "strsafe.h"
#include "SimpleMsg/SimpleMsg.h"

#include "EventLog.h"

/////////////////////////////////////////////////////////////////////////////
// CEventLog 

CEventLog* CEventLog::s_pEventLog = nullptr;	// static init
//DWORD CEventLog::m_dwDefaultMsgBase = 0;

CEventLog::CEventLog()
{
}

CEventLog::~CEventLog()
{
	if ( m_hEventLogWrite )
	{
		DeregisterEventSource( m_hEventLogWrite );
	}
	delete [] (BYTE*) m_pTokenUser;
}

CEventLog& CEventLog::GetInstance() {
	ASSERT( s_pEventLog != NULL );
	return *s_pEventLog;
}

void CEventLog::Init( 
	LPCTSTR lpSourceName,			// = NULL
	PSID	lpUserSid,				// = NULL
	LPCTSTR lpUNCServerName )		// = NULL
{
	if ( s_pEventLog == NULL )
		s_pEventLog = new CEventLog();
	ASSERT(s_pEventLog->m_nMsg == 0 && s_pEventLog->m_strSourceName.IsEmpty());

	if ( lpSourceName == NULL )
		s_pEventLog->m_strSourceName = AfxGetApp()->m_pszAppName;
	else
		s_pEventLog->m_strSourceName = lpSourceName;

	s_pEventLog->m_lpUserSid = lpUserSid;

	if ( lpUNCServerName != NULL ) 
		s_pEventLog->m_strUNCServerName = lpUNCServerName;

	if ( s_pEventLog->m_hEventLogWrite != NULL )
	{
		DeregisterEventSource( s_pEventLog->m_hEventLogWrite );
	}
	s_pEventLog->m_hEventLogWrite = RegisterEventSource( lpUNCServerName, s_pEventLog->m_strSourceName );

#ifdef _DEBUG
	if ( s_pEventLog->m_hEventLogWrite == NULL )
	{
		DWORD dwErr = GetLastError();
		ASSERT( FALSE );
	}
#endif
}

void CEventLog::InitMsg(UINT nMsg, CWnd* pTargetWnd)
{
	if (s_pEventLog == NULL)
		s_pEventLog = new CEventLog();
	ASSERT(s_pEventLog->m_nMsg == 0 && s_pEventLog->m_strSourceName.IsEmpty());
	s_pEventLog->m_nMsg = nMsg;
	s_pEventLog->m_pTargetWnd = pTargetWnd;
}

/////////////////////////////////////////////////////////////////////////////
// CEventLog commands

void CEventLog::Register( 
	LPCTSTR lpEventMessageFile,		// = NULL
	DWORD dwTypesSupported,			// = ...
	DWORD dwCategoryCount,			// = 0
	LPCTSTR lpCategoryMessageFile,	// = NULL
	LPCTSTR lpParameterMessageFile	// = NULL
)		// throws *CRegException
{
	ASSERT( ! m_strSourceName.IsEmpty() );
	if ( m_strSourceName.IsEmpty() ) return;

	TCHAR szPath[MAX_PATH];
	if( lpEventMessageFile == NULL )
	{
		DWORD dwLen = GetModuleFileName( NULL, szPath, MAX_PATH );
		if ( dwLen == 0 )
			return;
		LPTSTR lpFile = _tcsrchr( szPath, '\\' );
		if ( lpFile == NULL )
			return;
		_tcscpy_s( lpFile + 1, MAX_PATH-dwLen, _T("SimpleMsg.dll") );
		lpEventMessageFile = szPath;
	}

	CRegistry reg;

	reg.OpenSubKey( HKEY_LOCAL_MACHINE,
		_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"),
		KEY_CREATE_SUB_KEY );
	reg.CreateSubKey( m_strSourceName, KEY_SET_VALUE );
	reg.AddString( _T("EventMessageFile"), lpEventMessageFile, REG_EXPAND_SZ );
	reg.AddDword( _T("TypesSupported"), dwTypesSupported );
	if ( dwCategoryCount > 0 )
	{
		reg.AddDword( _T("CategoryCount"), dwCategoryCount );
		ASSERT( lpCategoryMessageFile != NULL );
		reg.AddString( _T("CategoryMessageFile"), lpCategoryMessageFile, REG_EXPAND_SZ );
	}
	if ( lpParameterMessageFile != NULL )
		reg.AddString( _T("ParameterMessageFile"), lpParameterMessageFile, REG_EXPAND_SZ );
	reg.CloseKey();
}


BOOL CEventLog::SetUserSid( PSID pUserSid /* = NULL */ )
{
	if ( pUserSid != NULL )
	{
		m_lpUserSid = pUserSid;
		return TRUE;
	}

	// get SID of current user

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hAccessToken;
	if ( ! OpenProcessToken( hProcess, TOKEN_READ, &hAccessToken ) )
		return FALSE;

	DWORD dwTokenUserSize = 0;
	GetTokenInformation( hAccessToken, TokenUser, NULL, 0, &dwTokenUserSize );
	if ( dwTokenUserSize == 0 )
		return FALSE;

	m_pTokenUser = (PTOKEN_USER) new BYTE[ dwTokenUserSize ];
	if ( ! GetTokenInformation( hAccessToken, TokenUser, m_pTokenUser,
		dwTokenUserSize, &dwTokenUserSize ) )
		return FALSE;

	m_lpUserSid = m_pTokenUser->User.Sid;
	return TRUE;
}

BOOL CEventLog::Write(
    WORD wType,			// event type to log 
    WORD wCategory,		// event category 
    DWORD dwEventID,	// event identifier 
    WORD wNumStrings,	// number of strings to merge with message  
    DWORD dwDataSize,	// size of binary data, in bytes
    LPCTSTR *lpStrings,	// array of strings to merge with message 
    LPVOID lpRawData 	// address of binary data 
)
{
	CEventLog &inst = GetInstance();
	if (inst.m_nMsg != 0 && wNumStrings >= 1)
	{
		CString* pTxt = new CString(lpStrings[0]);
		for (int n = 1; n < wNumStrings; n++)
		{
			*pTxt += '\n' + lpStrings[n];
		}
		WPARAM wT = NIIF_NONE;
		switch (wType)
		{
		case EVENTLOG_INFORMATION_TYPE:
			wT = NIIF_INFO;
			break;
		case EVENTLOG_WARNING_TYPE:
			wT = NIIF_WARNING;
			break;
		case EVENTLOG_ERROR_TYPE:
			wT = NIIF_ERROR;
			break;
		default:
			break;
		}
		ASSERT(inst.m_pTargetWnd->PostMessage(inst.m_nMsg, wT, (LPARAM)pTxt));
		return TRUE;
	}
	BOOL bRc = ReportEvent( 
		inst.m_hEventLogWrite,	// handle returned by RegisterEventSource 
		wType,				// event type to log
		wCategory,			// event category
		dwEventID,			// event identifier 
		inst.m_lpUserSid,		// user security identifier (optional)
		wNumStrings,		// number of strings to merge with message
		dwDataSize,			// size of binary data, in bytes
		lpStrings,			// array of strings to merge with message 
		lpRawData ); 		// address of binary data
#ifdef _DEBUG
	if ( !bRc )
	{
		DWORD dwErr = GetLastError();
		TRACE1("ReportEvent err=%d\n", dwErr);
		ASSERT( FALSE );
	}
#endif
	return bRc;
}

BOOL CEventLog::Write(
	CException* pe,		// ptr to exception
    DWORD dwEventID,	// = 0 generic event identifier 
	WORD wType, /* = EVENTLOG_ERROR_TYPE */
	WORD wCategory /* = 0 */
)
{
	if ( pe->IsKindOf(RUNTIME_CLASS(CEventLogException)))
		return ((CEventLogException*)pe)->ReportEvent( 
							wType, wCategory );

//	if ( dwEventID == 0 )
//		dwEventID = m_dwDefaultMsgBase;

	const UINT nMaxError = 256;
	_TCHAR szError[nMaxError];
	LPCTSTR lpStrings[ 3 ] = { szError };

	if ( ! pe->GetErrorMessage( szError, nMaxError ) )
		return FALSE;

	CRuntimeClass* pRunClass = pe->GetRuntimeClass();
//	lpStrings[1] = pRunClass->m_lpszClassName;
//	lpStrings[2] = CEventLogException::GetContext();
	
	return Write(
		wType,			// event type to log 
		wCategory,		// event category 
		dwEventID,		// event identifier 
		3,				// number of strings to merge with message  
		0,				// size of binary data, in bytes
		lpStrings,		// array of strings to merge with message 
		NULL );			// address of binary data 
}

BOOL CEventLog::Write(
    WORD wType,			// event type to log 
    WORD wCategory,		// event category 
    DWORD dwEventID,	// event identifier 
    CStringArray* paString,	// ptr to array of strings to merge with message
    DWORD dwDataSize,	// = 0	size of binary data, in bytes
    LPVOID lpRawData 	// = NULL	address of binary data 
)
{
	WORD wNumStrings = 0;
	const int nMaxStrings = 20;
	LPCTSTR alpString[ nMaxStrings ];

	if ( paString )
	{
		paString->AssertValid();
		wNumStrings = (WORD)paString->GetSize();
		ASSERT( wNumStrings < nMaxStrings );
		for ( int n = 0; n < wNumStrings; n++ )
		{
			alpString[ n ] = paString->GetAt( n );
#ifdef _DEBUG
			if ( n == 0 )
				TRACE2( "LogWrite(%d): %s\n", n, alpString[n] );
			else
				TRACE2( "        (%d): %s\n", n, alpString[n] );
#endif
		}
	}

	return Write( 
		wType,				// event type to log
		wCategory,			// event category
		dwEventID,			// event identifier 
		wNumStrings,		// number of strings to merge with message
		dwDataSize,			// size of binary data, in bytes
		alpString,			// array of strings to merge with message 
		lpRawData ) ;		// address of binary data
}

BOOL CEventLog::Write(
    WORD wType,			// event type to log 
    WORD wCategory,		// event category 
    DWORD dwEventID,	// event identifier 
    LPCTSTR pString,	// = NULL ptr to string to merge with message
    DWORD dwDataSize,	// = 0	size of binary data, in bytes
    LPVOID lpRawData 	// = NULL	address of binary data 
)
{
	WORD wNumStrings = 0;
	LPCTSTR alpString[ 1 ];

	if ( pString )
	{
		wNumStrings = 1;
		alpString[ 0 ] = pString;
#ifdef _DEBUG
		TRACE1( "LogWrite(): %s\n", pString );
#endif
	}

	return Write( 
		wType,				// event type to log
		wCategory,			// event category
		dwEventID,			// event identifier 
		wNumStrings,		// number of strings to merge with message
		dwDataSize,			// size of binary data, in bytes
		alpString,			// array of strings to merge with message 
		lpRawData ) ;		// address of binary data
}

//***************************************************************************

/////////////////////////////////////////////////////////////////////////////
// CEventLogException

IMPLEMENT_DYNAMIC(CEventLogException, CException)

CString CEventLogException::m_strContext;

CEventLogException::CEventLogException( DWORD dwMsg, LPCTSTR pszMsg /* = NULL */ )
{
	initMsg1( dwMsg, pszMsg );
}

CEventLogException::CEventLogException( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg /* = NULL */ )
{
	initMsg2( dwMsg, dwLastError, pszMsg );
}

CEventLogException::CEventLogException( LPCTSTR pszMsg /* = NULL */ )
{
//	DWORD dwMsg = CEventLog::GetMsgBase();
	DWORD dwMsg = MSG_INFO;
	DWORD dwLastError = GetLastError();

	initMsg2( dwMsg, dwLastError, pszMsg );
}

CEventLogException::~CEventLogException()
{
}

void CEventLogException::initMsg1( DWORD dwMsg, LPCTSTR pszMsg )
{
	m_dwMsg = dwMsg;

	if ( pszMsg == NULL )
		m_strMsg = "(null msg)";
	else
		m_strMsg = pszMsg;
}

void CEventLogException::initMsg2( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg )
{
	m_dwMsg = dwMsg;

	if ( pszMsg != NULL )
	{
		m_strMsg = pszMsg;
		if ( dwLastError != 0 )
			m_strMsg += ": ";
	}
	if ( dwLastError != 0 )
		m_strMsg += GetLastErrorText( dwLastError );
}


/////////////////////////////////////////////////////////////////////////////
// CEventLogException diagnostics

#ifdef _DEBUG
void CEventLogException::AssertValid() const
{
	CException::AssertValid();
}

void CEventLogException::Dump(CDumpContext& dc) const
{
	CException::Dump(dc);

	dc << "m_dwMsg = " << m_dwMsg << "\n";
	dc << "m_strMsg = " << m_strMsg << "\n";
	dc << "m_strContext = " << m_strContext << "\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEventLogException commands

BOOL CEventLogException::GetErrorMessage( 
		LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext )
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	if (nMaxError == 0 || lpszError == NULL)
		return FALSE;

	CString strMsg;
	if ( ! m_strContext.IsEmpty() )
		strMsg = m_strContext + _T(": ");
	strMsg += GetRuntimeClass()->m_lpszClassName;
	strMsg += _T(": ") + m_strMsg;

	StringCchCopy( lpszError, nMaxError, strMsg );
	return TRUE;
}

BOOL CEventLogException::ReportEvent( 
//		CEventLog* pEventLog,	// = NULL
		WORD wType,					// = EVENTLOG_ERROR_TYPE
		WORD wCategory )			// = 0
{
//	if ( pEventLog == NULL )
//		pEventLog = CEventLog::GetThis();
	CEventLog &eventLog = CEventLog::GetInstance();

	WORD wNumStrings = 3;
	LPCTSTR alpString[ 3 ];

	CString strClassName = CString(GetRuntimeClass()->m_lpszClassName);
	alpString[ 0 ] = m_strMsg;
	alpString[ 1 ] = strClassName;
	alpString[ 2 ] = m_strContext;

#ifdef _DEBUG
	CString strMsg;
	strMsg.Format( _T("CEventLogException::ReportEvent %s %s\n"), (LPCTSTR)m_strMsg, (LPCTSTR)m_strContext );
	TRACE( strMsg );
#endif

	return ( eventLog.Write( 
		wType,				// event type to log
		wCategory,			// event category
		m_dwMsg,			// event identifier 
		wNumStrings,		// number of strings to merge with message
		0,					// size of binary data, in bytes
		alpString,			// array of strings to merge with message 
		NULL ) );			// address of binary data 
}

CString CEventLogException::GetLastErrorText( DWORD dwLastError )
{
	CString strResult;
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM |
								FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL,
                           dwLastError,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), // Default language
                           (LPTSTR)&lpszTemp,
                           0,
                           NULL );
    if ( dwRet > 2 )
    {
        lpszTemp[lstrlen(lpszTemp)-2] = TEXT('\0');  //remove cr and newline character
    }
    strResult.Format( TEXT("%s (Err=%d)"), lpszTemp, dwLastError );

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    return strResult;
}



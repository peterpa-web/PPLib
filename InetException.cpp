// SniInetException.cpp: implementation of the CsniInetException class
//

#include "stdafx.h"
#include "InetException.h"
#include "wininet.h"

#pragma comment( lib,"wininet" )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CsniException

IMPLEMENT_DYNAMIC(CInetException, CEventLogException)

CInetException::CInetException( DWORD dwMsg, LPCTSTR pszMsg /* = NULL */ )
{
	DWORD dwLastError = GetLastError();

	initMsg2( dwMsg, dwLastError, pszMsg );
}

CInetException::CInetException( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg /* = NULL */ )
{
	initMsg2( dwMsg, dwLastError, pszMsg );
}

CInetException::CInetException( LPCTSTR pszMsg /* = NULL */, DWORD dwLastError /* = 0 */ )
{
	if ( dwLastError == 0 )
		dwLastError = GetLastError();
//	DWORD dwMsg = CEventLog::GetMsgBase();
	DWORD dwMsg = 0;

	initMsg2( dwMsg, dwLastError, pszMsg );
}

CInetException::~CInetException()
{
}

void CInetException::initMsg2( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg )
{
	m_dwMsg = dwMsg;
	m_nExtErrno = 0;

	if ( pszMsg != NULL )
	{
		m_strMsg = pszMsg;
		m_strMsg += ": ";
	}
	m_strMsg += GetLastErrorText( dwLastError );

	if ( dwLastError == ERROR_INTERNET_EXTENDED_ERROR )
	{
		int p = m_strMsg.GetLength() - 3;
		while ( --p > 0 )
		{
			if ( m_strMsg[p] == '\n' )
			{
				m_nExtErrno = _tstoi(m_strMsg.Mid(p+1));
				break;
			}
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
// CsniException diagnostics

#ifdef _DEBUG
void CInetException::AssertValid() const
{
	CEventLogException::AssertValid();
}

void CInetException::Dump(CDumpContext& dc) const
{
	CEventLogException::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CsniException commands

BOOL CInetException::GetErrorMessage( 
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
	strMsg += _T("CsniInetException: ") + m_strMsg;
	
	lstrcpyn( lpszError, strMsg, nMaxError );
	return TRUE;
}

BOOL CInetException::ReportEvent( 
//		CEventLog* pEventLog,	// = NULL
		WORD wType,					// = EVENTLOG_ERROR_TYPE
		WORD wCategory )			// = 0
{
//	if ( pEventLog == NULL )
//		pEventLog = CEventLog::GetThis();

	WORD wNumStrings = 3;
	LPCTSTR alpString[ 3 ];

	alpString[ 0 ] = m_strMsg;
	alpString[ 1 ] = _T("CsniInetException");
	alpString[ 2 ] = m_strContext;

	TRACE3( "LogWrite %s %s %s\n", alpString[ 1 ], alpString[ 2 ], alpString[ 0 ] );

	return (CEventLog::GetInstance().Write(
		wType,				// event type to log
		wCategory,			// event category
		m_dwMsg,			// event identifier 
		wNumStrings,		// number of strings to merge with message
		0,					// size of binary data, in bytes
		alpString,			// array of strings to merge with message 
		NULL ) );			// address of binary data 
}

CString CInetException::GetLastErrorText( DWORD dwLastError )
{
	if ( dwLastError < INTERNET_ERROR_BASE ||
		 dwLastError > INTERNET_ERROR_LAST )
		 return CEventLogException::GetLastErrorText( dwLastError );

	CString strResult;
    DWORD dwRet;
    LPTSTR lpszTemp = NULL;

    dwRet = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_HMODULE,
				GetModuleHandle(_T("wininet.dll")),
				dwLastError, 0,
				(LPTSTR)&lpszTemp, 0, NULL );
    if ( dwRet > 2 )
    {
        lpszTemp[lstrlen(lpszTemp)-2] = _T('\0');  //remove cr and newline character
    }
    strResult.Format( _T("%s (Err=%d)"), lpszTemp, dwLastError );

    if ( lpszTemp )
        LocalFree((HLOCAL) lpszTemp );

    if ( dwLastError == ERROR_INTERNET_EXTENDED_ERROR )
    {
		DWORD dwIntError;
		DWORD dwLength = 0;

        InternetGetLastResponseInfo( &dwIntError, NULL, &dwLength );
        if ( dwLength == 0)
		    return strResult;

		CString strExResult;
		LPTSTR pszBuf = strExResult.GetBuffer( dwLength + 1 );
        InternetGetLastResponseInfo( &dwIntError, pszBuf, &dwLength );

		strExResult.ReleaseBuffer();
		strResult += _T(":\r\n") + strExResult;
	}
	return strResult;
}



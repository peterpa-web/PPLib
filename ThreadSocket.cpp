// ThreadSocket.cpp : implementation of the CThreadSocket class
//

#include "stdafx.h"
#include "ServerSocket.h"
#include "ThreadSocket.h"

/////////////////////////////////////////////////////////////////////////////
// CThreadSocket Construction

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CThreadSocket::CThreadSocket()
{
	m_bAutoDelete = FALSE;
}

CThreadSocket::~CThreadSocket()
{
	TRACE1("(%d) CThreadSocket deleted\n", m_nConn);
}

BOOL CThreadSocket::StartThread( int nConn, SOCKADDR_IN* sockAddr )
{
	m_nConn = nConn;
//	m_strClientIP.Format( "%s", inet_ntoa( sockAddr->sin_addr ) );

//	CStringA strClientIP;
//	strClientIP.Format( "%s:%d", inet_ntoa( sockAddr->sin_addr ),
//									 ntohs( sockAddr->sin_port ) );

//	TRACE2( "(%d) CThreadSocket::StartThread client=%s\n", nConn, CString(strClientIP) );
	CString strClientIP;
	DWORD dwLen = 50;
	int iR = WSAAddressToString( (LPSOCKADDR)sockAddr, sizeof(SOCKADDR_IN), NULL, strClientIP.GetBuffer(dwLen), &dwLen );
	if ( iR == 0 ) {
		strClientIP.ReleaseBuffer(dwLen);
		TRACE2( "(%d) CThreadSocket::StartThread client=%s\n", nConn, strClientIP );
	}
	m_pfnThreadProc = NULL;
	m_pThreadParams = NULL;

	if ( !CreateThread( CREATE_SUSPENDED, 0, NULL ) )
	{
		return FALSE;
	}
//	VERIFY( SetThreadPriority( THREAD_PRIORITY_NORMAL ) );
	if ( ResumeThread() == (DWORD)-1 )
		return FALSE;

	return TRUE;
}

BOOL CThreadSocket::InitInstance()
{
	ASSERT_VALID(this);

	return TRUE;   // by default enter Run() function
}

// main running routine until thread exits
int CThreadSocket::Run()
{
	ASSERT_VALID(this);
	ASSERT( FALSE );		// not overloaded!
//	return ExitInstance();
	return 0;
}

int CThreadSocket::ExitInstance()
{
	ASSERT_VALID(this);
//	ASSERT(AfxGetApp() != this);
//  default will 'delete this'
//	int nResult = m_msgCur.wParam;  // returns the value from PostQuitMessage
//	return nResult;
	return 0;
}

BOOL CThreadSocket::GetExitCode( DWORD& dwExitCode )
{
	ASSERT_VALID(this);
	return GetExitCodeThread( m_hThread, &dwExitCode );
}

BOOL CThreadSocket::IsReady()
{
	if ( m_hSocket != INVALID_SOCKET )
		return FALSE;
	
	if ( m_hThread == NULL )
		return TRUE;

	DWORD dwExitCode;
	if ( GetExitCode( dwExitCode ) && 
		 dwExitCode == STILL_ACTIVE ) {
		TRACE1("(%d) CThreadSocket::IsReady() STILL_ACTIVE\n", m_nConn);
		return FALSE;
	}

	// free thread object
	CloseHandle( m_hThread );
	m_hThread = NULL;

	// cleanup module state
//	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
//	if (pState->m_pCurrentWinThread == this)
//		pState->m_pCurrentWinThread = NULL;

	return TRUE;
}

int CThreadSocket::SendString( LPCSTR lpsz )
{
//	TRACE2( "(%d) CThreadSocket::SendString Resp=%s\n", m_nConn, lpsz );
	return CTcpSocket::Send( lpsz ); 
}

int CThreadSocket::SendHTTP( CStringA str, int nStatus /* = 200 */, LPCSTR pszType /* ="text/html" */ )
{
	CStringA strHead;
	if ( nStatus == 200 )
	{
		strHead.Format("HTTP/1.1 200 OK\r\nPragma: no-cache\r\nContent-Type: %s\r\n", pszType);
		if (m_bKeepAlive)
			strHead += "Connection: Keep-Alive\r\n";
	}
	else
	{
		m_bKeepAlive = FALSE;
		if ( nStatus == 404 )
		{
			strHead.Format("HTTP/1.1 404 Not Found\r\nPragma: no-cache\r\nContent-Type: %s\r\n", pszType);
			if ( str.IsEmpty() )
				str = "<html><title>404 Not found</title><body>404 Not Found</body></html>\r\n";
		}
		else
		{
			strHead.Format("HTTP/1.1 400 Bad Request\r\nPragma: no-cache\r\nContent-Type: %s\r\n", pszType);
			if ( str.IsEmpty() )
				str = "<html><title>400 Bad Request</title><body>400 Bad Request</body></html>\r\n";
		}
		TRACE3( "(%d) CThreadSocket::SendHTTP nStatus=%d %s\n", m_nConn, nStatus, CString(str) );
	}
	int nLen = str.GetLength();
	if ( nLen > 0 )
		strHead.AppendFormat( "Content-Length: %d\r\n", nLen );
	strHead += "\r\n";

	if ( nStatus != 200 ) {
//		CsniEventLog::Write(EVENTLOG_WARNING_TYPE, 0, MSG_INFO, CString( "COsmThreadSocket::SendHTTP " + str ) );
	}

//	TRACE2("(%d) CThreadSocket::SendHTTP %s\n", m_nConn, CString(strHead + str));

	return Send( strHead + str ); 
}

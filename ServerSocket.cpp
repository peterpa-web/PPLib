// ServerSocket.cpp : implementation of the CServerSocket class
//

#include "stdafx.h"
#include "ThreadSocket.h"
#include "ServerSocket.h"

/////////////////////////////////////////////////////////////////////////////
// CServerSocket Construction

CServerSocket::CServerSocket()
{
	m_pevStop = NULL;
	m_pThread = NULL;
}

CServerSocket::~CServerSocket()
{
	if (m_pThread)
		WaitThreadFinished();
	delete m_pevStop;
}

void CServerSocket::AddThreadSocket( CThreadSocket* pTS )
{
	m_apThreadSocket.Add( pTS );
}

BOOL CServerSocket::Run()
{
	struct timeval		timeout;
	fd_set				readfds;

	ASSERT( m_apThreadSocket.GetSize() > 0 );
	if (m_pevStop == NULL)
		m_pevStop = new CEvent(FALSE, TRUE);
	ASSERT( m_pevStop != NULL );

	while ( TRUE )
	{
		CThreadSocket* pTH = NULL;
		int nConn;

		// freien ThreadSocket suchen
		int nTH = (int)m_apThreadSocket.GetSize();
		for ( nConn = 0; nConn < nTH; nConn++ )
		{
			pTH = GetThreadSocket( nConn );
			if ( pTH->IsReady() )
			{
				FTRACE1( "CServerSocket::Run using free socket %d\n", nConn );
				break;
			}

			pTH = NULL;
		}

		if ( pTH == NULL )
		{
			// warten auf freien ThreadSocket

			HANDLE* paEvHandles = new HANDLE[ nTH+1 ];
			paEvHandles[ 0 ] = *m_pevStop;
			for ( int n = 1; n <= nTH; n++ )
			{
				paEvHandles[ n ] = GetThreadSocket( n-1 )->GetThreadHandle();
			}
			FTRACE0( "CServerSocket::Run waiting for socket\n" );
			DWORD dwLock = WaitForMultipleObjects( nTH+1, paEvHandles, FALSE, INFINITE );
			if ( dwLock == WAIT_FAILED )
			{
				FTRACE0( "CServerSocket::Run wait failed\n" );
				delete [] paEvHandles;
				return FALSE;
			}
			if ( dwLock == WAIT_OBJECT_0 )
			{
				FTRACE0( "CServerSocket::Run wait stopped\n" );
				delete [] paEvHandles;
				return TRUE;		// Server Stopped
			}

			nConn = dwLock - WAIT_OBJECT_0 - 1;
			FTRACE1( "CServerSocket::Run using ended socket %d\n", nConn );
			pTH = GetThreadSocket( nConn );
			VERIFY( pTH->IsReady() );	// schlie�t ggf. alte thread

			delete [] paEvHandles;
		}

		// auf Verbindung warten

		FD_ZERO (&readfds);
		FD_SET ( m_hSocket, &readfds );        //  SOCKET einstellen
 		timeout.tv_sec  = 0;
		timeout.tv_usec = 0;              
 
		int iRc = select( 0, &readfds, NULL, NULL, &timeout );
		if ( iRc == SOCKET_ERROR ) 
		{
			SetLastError( WSAGetLastError() );
			return FALSE;
		}

		SOCKADDR sockAddr;
		memset(&sockAddr,0,sizeof(sockAddr));
		int nLenSockAddr = sizeof(sockAddr);

		if ( ! Accept( *pTH, &sockAddr, &nLenSockAddr ) )
		{
			DWORD dwProblem = ::GetLastError();
			SetLastError(dwProblem);
			if ( dwProblem == WSAEINTR )	// server stopped via closesocket
				break;

			return FALSE;
		}

		pTH->StartThread( nConn, (SOCKADDR_IN*)&sockAddr );
	}

	// stopped
	return TRUE;
}


CThreadSocket* CServerSocket::TerminateThreadSockets()
{
	// returns NULL if OK or ptr to busy ThreadSocket

	int nTH = (int)m_apThreadSocket.GetSize();
	for ( int n = nTH - 1; n >= 0; n-- )
	{
		CThreadSocket* pTH = GetThreadSocket( n );
		if ( ! pTH->IsReady() )
		{
			FTRACE1( "CServerSocket::TerminateThreadSockets returning busy socket %d\n", n );
			return pTH;
		}
		delete pTH;
		m_apThreadSocket.RemoveAt( n );
	}
	return NULL;
}

BOOL CServerSocket::BeginThread(int nPort)
{
	FTRACE1("CServerSocket::BeginThread nPort=%d\n", nPort);
	if (!Create(nPort)) {
		FTRACE0("CServerSocket::BeginThread Create failed\n");
		return FALSE;
	}
	if (!Listen()) {
		FTRACE0("CServerSocket::BeginThread Listen failed\n");
		return FALSE;
	}
	m_pThread = AfxBeginThread(&ThreadProc, this);
	m_pThread->m_bAutoDelete = FALSE;
	TRACE0("CServerSocket::BeginThread\n");
	return TRUE;
}

UINT CServerSocket::ThreadProc(LPVOID pParam)
{
	CServerSocket* pThis = (CServerSocket*)pParam;
	return pThis->ThreadProcInt();
}

UINT CServerSocket::ThreadProcInt()
{
	BOOL bResult = Run();
	ASSERT(bResult);

	FTRACE0("CServerSocket::ThreadProcInt cleanup\n");
	for (int i = 0; i < m_apThreadSocket.GetSize(); ++i)
	{
		GetThreadSocket(i)->Stop();
	}
	Sleep(100);
	CThreadSocket*pTH = TerminateThreadSockets();
	int nLimit = m_apThreadSocket.GetSize() * 10;	// 10*100 ms per client
	while (pTH != NULL)
	{
		pTH->ShutDown(2);
		Sleep(100);
		pTH->Close();
		pTH = TerminateThreadSockets();
		if (--nLimit < 0) {
			FTRACE0("CServerSocket::ThreadProcInt cleanup Timeout TerminateThreadSockets\n");
//			CsniEventLog::Write(EVENTLOG_WARNING_TYPE, 0, MSG_INFO, _T("Timeout TerminateThreadSockets"));
			break;
		}
	}
	return 1;
}

void CServerSocket::WaitThreadFinished()
{
	if (m_pThread == NULL)
		return;

	FTRACE0("CServerSocket::WaitThreadFinished\n");
	if (m_pevStop != NULL)
		m_pevStop->SetEvent();

	ShutDown(2);
	Sleep(100);
	Close();
	VERIFY(::WaitForSingleObject(m_pThread->m_hThread, INFINITE) == WAIT_OBJECT_0);
	delete m_pThread;
	m_pThread = NULL;
	FTRACE0("CServerSocket::WaitThreadFinished done\n");
}

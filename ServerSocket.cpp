// ServerSocket.cpp : implementation of the CServerSocket class
//

#include "stdafx.h"
#include "ThreadSocket.h"
#include "ServerSocket.h"

/////////////////////////////////////////////////////////////////////////////
// CServerSocket Construction

CServerSocket::CServerSocket()
{
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

	m_bStop = false;
	if (m_pevStop == NULL)
		m_pevStop = new CEvent(FALSE, TRUE);
	ASSERT( m_pevStop != NULL );

	while (!m_bStop)
	{
		int nTH = (int)m_apThreadSocket.GetSize();
		if (nTH < 1)
		{
			FTRACE0("Run missing threads\n");
			return TRUE;
		}
		if (m_bPause)
		{
			Sleep(1000);
			continue;
		}

		CThreadSocket* pTH = NULL;
		int nConn;

		// freien ThreadSocket suchen
		for ( nConn = 0; nConn < nTH; nConn++ )
		{
			pTH = GetThreadSocket( nConn );
			if ( pTH->IsReady() )
			{
				FTRACE1( "Run using free socket %d\n", nConn );
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
			FTRACE0( "Run waiting for socket\n" );
			DWORD dwLock = WaitForMultipleObjects( nTH+1, paEvHandles, FALSE, INFINITE );
			if ( dwLock == WAIT_FAILED )
			{
				FTRACE0( "Run wait failed\n" );
				delete [] paEvHandles;
				return FALSE;
			}
			if ( dwLock == WAIT_OBJECT_0 )
			{
				FTRACE0( "Run wait stopped\n" );
				delete [] paEvHandles;
				return TRUE;		// Server Stopped
			}

			nConn = dwLock - WAIT_OBJECT_0 - 1;
			FTRACE1( "Run using ended socket %d\n", nConn );
			pTH = GetThreadSocket( nConn );
			VERIFY( pTH->IsReady() );	// schließt ggf. alte thread

			delete [] paEvHandles;
		}

		// auf Verbindung warten

		int iSel = 0;
		while (iSel == 0)
		{
			FD_ZERO(&readfds);
			FD_SET(m_hSocket, &readfds);        //  SOCKET einstellen
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			iSel = select(0, &readfds, NULL, NULL, &timeout);
			if (iSel == SOCKET_ERROR)
			{
				SetLastError(WSAGetLastError());
				return FALSE;
			}
			if (m_bPause || m_bStop)
				break;
		}
		if (m_bPause || m_bStop)
			continue;

		SOCKADDR sockAddr;
		memset(&sockAddr,0,sizeof(sockAddr));
		int nLenSockAddr = sizeof(sockAddr);

		if ( ! Accept( *pTH, &sockAddr, &nLenSockAddr ) )
		{
			DWORD dwProblem = ::GetLastError();
			SetLastError(dwProblem);
			if ( dwProblem == WSAEINTR )	// server stopped via closesocket
				break;

			FTRACE1("Run Accept=%d\n", dwProblem);
			return FALSE;
		}

		pTH->StartThread( nConn, (SOCKADDR_IN*)&sockAddr );
	}

	FTRACE0("Run finished\n");
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
			FTRACE1( "TerminateThreadSockets returning busy socket %d\n", n );
			return pTH;
		}
		delete pTH;
		m_apThreadSocket.RemoveAt( n );
	}
	return NULL;
}

BOOL CServerSocket::BeginThread(int nPort)
{
	FTRACE1("BeginThread nPort=%d\n", nPort);
	if (!Create(nPort)) {
		FTRACE0("BeginThread Create failed\n");
		return FALSE;
	}
	if (!Listen()) {
		FTRACE0("BeginThread Listen failed\n");
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
//	ASSERT(bResult);

	FTRACE0("ThreadProcInt cleanup\n");
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
			FTRACE0("ThreadProcInt cleanup Timeout TerminateThreadSockets\n");
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

	FTRACE0("WaitThreadFinished\n");
	Stop();

	ShutDown(2);
	Sleep(100);
	Close();
	VERIFY(::WaitForSingleObject(m_pThread->m_hThread, INFINITE) == WAIT_OBJECT_0);
	delete m_pThread;
	m_pThread = NULL;
	FTRACE0("WaitThreadFinished done\n");
}

bool CServerSocket::Pause(bool b)
{
	m_bPause = b;
	FTRACE1("Pause %d\n", b);
	if (!b)
		return true;

	for (int i = 0; i < m_apThreadSocket.GetSize(); ++i)
	{
		if (!GetThreadSocket(i)->IsReady())
			return false;
	}
	return true;
}

void CServerSocket::Stop()
{
	m_bStop = true;
	if (m_pevStop != NULL)
		m_pevStop->SetEvent();
}
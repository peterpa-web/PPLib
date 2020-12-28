// SimpleThread.cpp: implementation of the CSimpleThread class
//

#include "stdafx.h"
#include "SimpleThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleThread 

IMPLEMENT_DYNAMIC( CSimpleThread, CSimpleThread_Base )

CSimpleThread::CSimpleThread()
{
	m_bAutoDelete = FALSE;
	m_pevStop = NULL;
}

CSimpleThread::~CSimpleThread()
{
	if ( m_hThread )
		WaitUntilFinished();
	delete m_pevStop;
}


/////////////////////////////////////////////////////////////////////////////
// CSimpleThread diagnostics

#ifdef _DEBUG
void CSimpleThread::AssertValid() const
{
	CSimpleThread_Base::AssertValid();
}

void CSimpleThread::Dump(CDumpContext& dc) const
{
	CSimpleThread_Base::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSimpleThread commands

BOOL CSimpleThread::BeginThread(
			AFX_THREADPROC pfnThreadProc,		// = NULL
			LPVOID pParam,						// = NULL
			int nPriority,						// = THREAD_PRIORITY_NORMAL
			UINT nStackSize,					// = 0
			DWORD dwCreateFlags,				// = 0
			LPSECURITY_ATTRIBUTES lpSecurityAttrs)	// = NULL
{
	m_pfnThreadProc = pfnThreadProc;
	m_pThreadParams = pParam;

	if ( m_pevStop == NULL )
		m_pevStop = new CEvent( FALSE, TRUE );

	if ( !CreateThread( dwCreateFlags|CREATE_SUSPENDED, nStackSize,
						lpSecurityAttrs) )
	{
		return FALSE;
	}
	VERIFY( SetThreadPriority( nPriority ) );
	if ( !(dwCreateFlags & CREATE_SUSPENDED) )
		VERIFY( ResumeThread() != (DWORD)-1 );

	return TRUE;
}

BOOL CSimpleThread::InitInstance()
{
	ASSERT_VALID(this);

	return TRUE;   // by default enter Run() function
}

// main running routine until thread exits
int CSimpleThread::Run()
{
	ASSERT_VALID(this);
	ASSERT( FALSE );		// not overloaded!
//	return ExitInstance();
	return 0;
}

int CSimpleThread::ExitInstance()
{
	ASSERT_VALID(this);
//	ASSERT(AfxGetApp() != this);

//	int nResult = m_msgCur.wParam;  // returns the value from PostQuitMessage
//	return nResult;
	return 0;
}

void CSimpleThread::StopThread()
{
	if ( m_pevStop != NULL )
		m_pevStop->SetEvent();
}

BOOL CSimpleThread::GetExitCode( DWORD& dwExitCode )
{
	ASSERT_VALID(this);
	return GetExitCodeThread( m_hThread, &dwExitCode );
}

void CSimpleThread::WaitUntilFinished()
{
	if ( m_hThread == NULL )
		return;

	StopThread();

	VERIFY(::WaitForSingleObject(m_hThread, INFINITE) == WAIT_OBJECT_0);
	::CloseHandle(m_hThread);
	m_hThread = NULL;
}


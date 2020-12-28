// SimpleThread.h: interface of the CSimpleThread class
//

#pragma once
#include <afxmt.h>

#define CSimpleThread_Base CWinThread

class CSimpleThread : public CSimpleThread_Base
{
protected: 

// Attributes
public:
	CSimpleThread();
	DECLARE_DYNAMIC( CSimpleThread )
	CEvent* m_pevStop;

// Operations
public:
	BOOL BeginThread(AFX_THREADPROC pfnThreadProc = NULL, LPVOID pParam = NULL,
			int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
			DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	void StopThread();

	BOOL GetExitCode( DWORD& dwExitCode );

	void WaitUntilFinished();

// Overridables
	// thread initialization
	virtual BOOL InitInstance();

	// running and idle processing
	virtual int Run();

	// thread termination
	virtual int ExitInstance(); // default will 'delete this'

// Implementation
public:
	virtual ~CSimpleThread();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

};



/*
class CWinThread : public CCmdTarget
{
	DECLARE_DYNAMIC(CWinThread)

public:
// Constructors
	CWinThread();
	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

// Attributes
	CWnd* m_pMainWnd;       // main window (usually same AfxGetApp()->m_pMainWnd)
	CWnd* m_pActiveWnd;     // active main window (may not be m_pMainWnd)
	BOOL m_bAutoDelete;     // enables 'delete this' after thread termination

	// only valid while running
	HANDLE m_hThread;       // this thread's HANDLE
	operator HANDLE() const;
	DWORD m_nThreadID;      // this thread's ID

	int GetThreadPriority();
	BOOL SetThreadPriority(int nPriority);

// Operations
	DWORD SuspendThread();
	DWORD ResumeThread();
	BOOL PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam);

// Overridables
	// thread initialization
	virtual BOOL InitInstance();

	// running and idle processing
	virtual int Run();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL PumpMessage();     // low level message pump
	virtual BOOL OnIdle(LONG lCount); // return TRUE if more idle processing
	virtual BOOL IsIdleMessage(MSG* pMsg);  // checks for special messages

	// thread termination
	virtual int ExitInstance(); // default will 'delete this'

	// Advanced: exception handling
	virtual LRESULT ProcessWndProcException(CException* e, const MSG* pMsg);

	// Advanced: handling messages sent to message filter hook
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

	// Advanced: virtual access to m_pMainWnd
	virtual CWnd* GetMainWnd();

// Implementation
public:
	virtual ~CWinThread();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	int m_nDisablePumpCount; // Diagnostic trap to detect illegal re-entrancy
#endif
	void CommonConstruct();
	virtual void Delete();
		// 'delete this' only if m_bAutoDelete == TRUE

	// message pump for Run
	MSG m_msgCur;                   // current message

public:
	// constructor used by implementation of AfxBeginThread
	CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);

	// valid after construction
	LPVOID m_pThreadParams; // generic parameters passed to starting function
	AFX_THREADPROC m_pfnThreadProc;

	// set after OLE is initialized
	void (AFXAPI* m_lpfnOleTermOrFreeLib)(BOOL, BOOL);
	COleMessageFilter* m_pMessageFilter;

protected:
	CPoint m_ptCursorLast;      // last mouse position
	UINT m_nMsgLast;            // last mouse message
	void DispatchThreadMessage(MSG* msg);  // helper
};

// global helpers for threads

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
CWinThread* AFXAPI AfxBeginThread(CRuntimeClass* pThreadClass,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

CWinThread* AFXAPI AfxGetThread();
void AFXAPI AfxEndThread(UINT nExitCode, BOOL bDelete = TRUE);

void AFXAPI AfxInitThread();
void AFXAPI AfxTermThread(HINSTANCE hInstTerm = NULL);


*/
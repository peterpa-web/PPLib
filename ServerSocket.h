// ServerSocket.h : interface of the CServerSocket class
//
#pragma once
#include <afxmt.h>
#include "SimpleSocket.h"

class CThreadSocket;
class CEvent;

class CServerSocket : public CTcpSocket
{
public:
	CServerSocket();
	virtual ~CServerSocket();

	void AddThreadSocket( CThreadSocket* pTS );
	CThreadSocket* GetThreadSocket( int n ) { return (CThreadSocket*)(m_apThreadSocket[ n ]); }

	void SetStopEvent( CEvent* pevStop ) { m_pevStop = pevStop; }

	BOOL Run();
	CThreadSocket* TerminateThreadSockets();

	BOOL BeginThread(int nPort);	// optional
	void WaitThreadFinished();

protected:
	CPtrArray m_apThreadSocket;
	CEvent* m_pevStop;
	CWinThread *m_pThread;
	static UINT ThreadProc(LPVOID pParam);
	UINT ThreadProcInt();
};


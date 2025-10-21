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

//	void SetStopEvent( CEvent* pevStop ) { m_pevStop = pevStop; }


	BOOL BeginThread(int nPort);	// optional
	void WaitThreadFinished();
	bool Pause(bool b);
	void Stop();

protected:
	BOOL Run();
	CPtrArray m_apThreadSocket;
	CEvent* m_pevStop = nullptr;
	CWinThread *m_pThread = nullptr;
	static UINT ThreadProc(LPVOID pParam);
	UINT ThreadProcInt();
	CThreadSocket* TerminateThreadSockets();
	bool m_bPause = false;
	bool m_bStop = false;
};


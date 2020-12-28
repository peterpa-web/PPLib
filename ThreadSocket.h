// ThreadSocket.h : interface of the CThreadSocket class
//
#pragma once
#include "SimpleSocket.h"

class CThreadSocket : public CTcpSocket, public CWinThread
{
public:
	CThreadSocket();
	virtual ~CThreadSocket();


	BOOL StartThread( int nConn, SOCKADDR_IN* sockAddr );

// Overridables
	// thread initialization
	virtual BOOL InitInstance();

	// running and idle processing
	virtual int Run();

	// thread termination
	virtual int ExitInstance();
	
	BOOL GetExitCode( DWORD& dwExitCode );
	BOOL IsReady();

	HANDLE GetThreadHandle() { return m_hThread; }

	void Stop() { m_bStop = TRUE; }
	BOOL IsStopped() const { return m_bStop; }

	int GetConn() const { return m_nConn; }

	int SendString( LPCSTR lpsz );
	int SendHTTP( CStringA str, int nStatus = 200, LPCSTR pszType="text/html" );

protected:
	int	m_nConn;
	BOOL m_bStop;
	BOOL m_bKeepAlive;

};


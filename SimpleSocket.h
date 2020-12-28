// SimpleSocket.h : interface of the CSimpleSocket class
//

#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <afxsock.h>		// MFC socket extensions

class CSimpleSocket
{
public:
	CSimpleSocket();
	virtual ~CSimpleSocket();

	operator SOCKET() const
		{ return m_hSocket; }

	BOOL Create(UINT nSocketPort = 0, int nSocketType=SOCK_STREAM,
		LPCTSTR lpszSocketAddress = NULL);

	BOOL Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress = NULL);

	virtual void Close();

	BOOL IsClosed() { return (m_hSocket == INVALID_SOCKET); }

	BOOL Connect(LPCSTR lpszHostAddress, UINT nHostPort);

	BOOL SetReceiveTimeout(DWORD nMilliSec);
	void SetSendTimeout( DWORD nMilliSec ) { m_nMilliSecSendTimeOut = nMilliSec; }

	CString GetLastError(int iRc, int nExpected);

protected:
	BOOL Init();
	BOOL Exit();

	static int g_nInit;
	SOCKET	m_hSocket;
	DWORD	m_nMilliSecRcvTimeOut;
	DWORD	m_nMilliSecSendTimeOut;
};

class CTcpSocket : public CSimpleSocket
{
	// using:
	// Create, Connect, Send, Receive, Close
	// Create, Listen, select, Accept, Receive, Send, Close

public:
	virtual BOOL Accept(CTcpSocket& rConnectedSocket,
					SOCKADDR* lpSockAddr = NULL, int* lpSockAddrLen=NULL );

	BOOL Listen(int nConnectionBacklog=5)
		{ return (SOCKET_ERROR != listen(m_hSocket, nConnectionBacklog)); }

	enum { receives = 0, sends = 1, both = 2 };
	BOOL ShutDown(int nHow = sends)
		{ return (SOCKET_ERROR != shutdown(m_hSocket,nHow)); }

	virtual int Receive(void* lpBuf, int nBufLen, int nFlags = 0);
	int Receive( CStringA& str, int nFlags = 0 );
	int ReceiveBlk(LPSTR lpBuf, int nBufLen, int nFlags = 0);
	int ReceiveHeader( CArray<CStringA>& arrLines, LPCSTR pszVersion = NULL );
	int ReceiveBlkX(LPSTR lpBuf, int nBufLen, int nFlags = 0);
	virtual int Send(const void* lpBuf, int nBufLen, int nFlags = 0);
	int Send( LPCSTR lpsz ) { return Send( lpsz, (int)strlen(lpsz) ); }
	int SendTerm( LPCSTR lpsz ) { return Send( lpsz, (int)strlen(lpsz)+1 ); }

protected:
	CStringA m_strExtra;
};

class CUdpSocket : public CSimpleSocket
{
public:
	CUdpSocket();
	virtual ~CUdpSocket();
	enum AddrType { Multi, Bind, IF };
	BOOL Create(); // socket Multi & bind
	BOOL Create( const char *addr, const char *port, int af, int type, int proto );
	int ReceiveFrom(void* lpBuf, int nBufLen, int nFlags, SOCKADDR *safrom, int *fromlen);
	int SendTo(const void* lpBuf, int nBufLen, int nFlags);
	int SendTo( LPCSTR lpsz ) { return SendTo( lpsz, (int)strlen(lpsz), 0 ); }

	BOOL JoinMulticastGroup();
	BOOL SetSendInterface();
	int FormatAddress(SOCKADDR *sa, int salen, char *addrbuf, int addrbuflen);
	int ResolveAddress(enum AddrType addrType, const char *addr, const char *port, int af, int type, int proto);
	int ResolveAddress(enum AddrType addrType, const char *addr, const char *port) { 
		return ResolveAddress( addrType, addr, port, m_pAddrMulti->ai_family, m_pAddrMulti->ai_socktype, m_pAddrMulti->ai_protocol); 
	}

protected:
	struct addrinfo *m_pAddrMulti;
	struct addrinfo *m_pAddrBind;
	struct addrinfo *m_pAddrIF;
};
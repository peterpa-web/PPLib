// SimpleSocket.cpp : implementation of the CSimpleSocket class
//

#include "stdafx.h"
#include "SimpleSocket.h"

#define SOCKETS_STARTUP_VERSION MAKEWORD( 2, 0 )
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleSocket Construction

int CSimpleSocket::g_nInit = 0;

CSimpleSocket::CSimpleSocket()
{
	m_hSocket = INVALID_SOCKET;
	m_nMilliSecRcvTimeOut = 30000;
	m_nMilliSecSendTimeOut = 30000;
	VERIFY( Init() );
}

CSimpleSocket::~CSimpleSocket()
{
	if (m_hSocket != INVALID_SOCKET)
		Close();
	VERIFY( Exit() );
}

BOOL CSimpleSocket::Init()
{
	if ( ++g_nInit > 1 )
		return TRUE;

	WSADATA wsaData;
    int iRc;

	// Socket-DLL initialisieren

	if ( iRc = WSAStartup( SOCKETS_STARTUP_VERSION, &wsaData ) ) 
	{
		Exit();
		return( FALSE );
	}
	return TRUE;
}

BOOL CSimpleSocket::Exit()
{
	ASSERT( g_nInit > 0 );
	if ( --g_nInit > 0 )
		return TRUE;

    int iRc;

	if ( iRc = WSACleanup() ) 
	{
		// WSAGetLastError() ;
		return( FALSE );
	}
	return TRUE;
}

BOOL CSimpleSocket::Create(UINT nSocketPort, int nSocketType,
	LPCTSTR lpszSocketAddress)
{
	ASSERT(m_hSocket == INVALID_SOCKET);

	m_hSocket = socket( AF_INET, nSocketType, 0 );
	if (m_hSocket == INVALID_SOCKET)
	{
		return FALSE;
	}

	if ( Bind( nSocketPort, lpszSocketAddress ) )
		return TRUE;

	int nResult = ::GetLastError();
	Close();
	WSASetLastError(nResult);

	return FALSE;
}

BOOL CSimpleSocket::Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress)
{
	USES_CONVERSION;

	SOCKADDR_IN sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));

//	LPSTR lpszAscii = T2A((LPTSTR)lpszSocketAddress);
	sockAddr.sin_family = AF_INET;

	if (lpszSocketAddress == NULL)	// lpszAscii ?
		sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
	{
//		DWORD lResult = inet_addr(lpszAscii);
//		if (lResult == INADDR_NONE)
//		{
//			WSASetLastError(WSAEINVAL);
//			return FALSE;
//		}
//		sockAddr.sin_addr.s_addr = lResult;
		ASSERT( FALSE );	// not tested:
	//	int iR = InetPton( sockAddr.sin_family, lpszSocketAddress, &sockAddr.sin_addr );
	//	if ( iR != 1 )
	//		return FALSE;
	}

	sockAddr.sin_port = htons((u_short)nSocketPort);

	return (SOCKET_ERROR != bind( m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr) ) );
}

void CSimpleSocket::Close()
{
	TRACE0("CSimpleSocket::Close()\n");
	if (m_hSocket != INVALID_SOCKET)
	{
		SOCKET hSocket = m_hSocket;		// Reihenfolge wegen Multi-Thread!
		m_hSocket = INVALID_SOCKET;
		if ( closesocket(hSocket) == SOCKET_ERROR )
		{
			TRACE1("CSimpleSocket::Close() failed %d\n", WSAGetLastError() );
		}
	}
}

BOOL CSimpleSocket::SetReceiveTimeout(DWORD nMilliSec) 
{ 
	m_nMilliSecRcvTimeOut = nMilliSec; 
	if (IsClosed())
		return TRUE;
	if (setsockopt(m_hSocket, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&m_nMilliSecRcvTimeOut,
		sizeof(m_nMilliSecRcvTimeOut)) == SOCKET_ERROR)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CSimpleSocket::Connect(LPCSTR lpszHostAddress, UINT nHostPort)
{
//	USES_CONVERSION;

	ASSERT(lpszHostAddress != NULL);

    // set timeouts
    if ( setsockopt (m_hSocket, SOL_SOCKET, SO_RCVTIMEO,
				(const char*)&m_nMilliSecRcvTimeOut, 
				sizeof(m_nMilliSecRcvTimeOut) ) == SOCKET_ERROR )
	{
		return FALSE;
	}
    if ( setsockopt (m_hSocket, SOL_SOCKET, SO_SNDTIMEO,
				(const char*)&m_nMilliSecSendTimeOut, 
				sizeof(m_nMilliSecSendTimeOut) ) == SOCKET_ERROR )
	{
		return FALSE;
	}

//	SOCKADDR_IN sockAddr;
//	memset(&sockAddr,0,sizeof(sockAddr));
	CStringA strPort;
	strPort.Format( "%ud", nHostPort );
	SOCKADDR_IN *psa_ipv4 = NULL;;
	struct addrinfo *paiResult = NULL;
	struct addrinfo aiHints;
	memset(&aiHints, 0, sizeof (aiHints));
    aiHints.ai_family = AF_UNSPEC;
    aiHints.ai_socktype = SOCK_STREAM;
    aiHints.ai_protocol = IPPROTO_TCP;
	int nRet = getaddrinfo( lpszHostAddress, strPort, &aiHints, &paiResult );
	if ( nRet != 0 )
		return FALSE;

	struct addrinfo *pai;
	for (pai = paiResult; pai != NULL; pai = pai->ai_next) {
		if ( pai->ai_family == AF_INET ) {
			psa_ipv4 = (struct sockaddr_in *) pai->ai_addr;
		}
	}
	freeaddrinfo(paiResult);
	if ( psa_ipv4 == NULL ) {
		WSASetLastError(WSAEINVAL);
		return FALSE;
	}

//	LPSTR lpszAscii = T2A((LPTSTR)lpszHostAddress);
//	sockAddr.sin_family = AF_INET;
//	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

//	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
//	{
//		LPHOSTENT lphost;
//		lphost = gethostbyname(lpszAscii);
//		if (lphost != NULL)
//			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
//		else
//		{
//			WSASetLastError(WSAEINVAL);
//			return FALSE;
//		}
//	}

//	sockAddr.sin_port = htons((u_short)nHostPort);

//	if ( connect(m_hSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR )
//		return FALSE;

	int iR = connect(m_hSocket, (SOCKADDR*)psa_ipv4, sizeof(SOCKADDR_IN));
	return (iR == 0);
}

CString CSimpleSocket::GetLastError(int iRc, int nExpected)
{
	CString str;
	if (iRc == SOCKET_ERROR)
	{
		iRc = WSAGetLastError();
		if (iRc == WSAETIMEDOUT)
			str = "WSAETIMEDOUT";
		else if (iRc == WSAECONNRESET)
			str = "WSAECONNRESET";
		else
			str.Format(_T("iRc=%d"), iRc);
	}
	str.Format(_T("%s missing=%d"), (LPCTSTR)str, nExpected);
	return str;
}

// ################################

BOOL CTcpSocket::Accept(CTcpSocket& rConnectedSocket,
	SOCKADDR* lpSockAddr, int* lpSockAddrLen)
{
	ASSERT(rConnectedSocket.m_hSocket == INVALID_SOCKET);

	SOCKET hTemp = accept(m_hSocket, lpSockAddr, lpSockAddrLen);

	if (hTemp == INVALID_SOCKET)
	{
		DWORD dwProblem = ::GetLastError();
		rConnectedSocket.m_hSocket = INVALID_SOCKET;
		SetLastError(dwProblem);
		return FALSE;
	}

	rConnectedSocket.m_hSocket = hTemp;

	    // set timeouts
    if ( setsockopt (hTemp, SOL_SOCKET, SO_RCVTIMEO,
				(const char*)&rConnectedSocket.m_nMilliSecRcvTimeOut, 
				sizeof(m_nMilliSecRcvTimeOut) ) == SOCKET_ERROR )
	{
		return FALSE;
	}
    if ( setsockopt (hTemp, SOL_SOCKET, SO_SNDTIMEO,
				(const char*)&rConnectedSocket.m_nMilliSecSendTimeOut, 
				sizeof(m_nMilliSecSendTimeOut) ) == SOCKET_ERROR )
	{
		return FALSE;
	}


	return (hTemp != INVALID_SOCKET);
}

int CTcpSocket::Receive(void* lpBuf, int nBufLen, int nFlags)
{
	int iRc = recv(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
//	TRACE1( "CSimpleSocket::Receive ret=%d\n", iRc );
	return iRc;
}

int CTcpSocket::Receive(CStringA& str, int nFlags)
{
	if ( !m_strExtra.IsEmpty() )
	{
		str = m_strExtra;
		m_strExtra.Empty();
		TRACE1( "CSimpleSocket::Receive str extra ret=%d\n", str.GetLength() );
		return str.GetLength();
	}

	int nLen = 8192;
	LPSTR lpBuf = str.GetBuffer( nLen );
	LPSTR lpB = lpBuf;
	int iRc = Receive( lpB, nLen, nFlags );
//	TRACE1( "CSimpleSocket::Receive str ret=%d\n", iRc );
	if ( iRc == SOCKET_ERROR || iRc == 0 )
	{
		str.ReleaseBufferSetLength( 0 );
		return iRc;
	}
	str.ReleaseBufferSetLength( iRc );
	return iRc;
}

int CTcpSocket::ReceiveBlk( LPSTR lpBuf, int nBufLen, int nFlags)
{
	int nLen = nBufLen;
	int iRc = 0;

	while ( nLen > 0 )
	{  
		iRc = Receive( lpBuf, nLen, nFlags );
		if ( iRc == SOCKET_ERROR || iRc == 0 )
		{
			TRACE1( "CSimpleSocket::ReceiveBlk err read=%d\n", nBufLen - nLen );
			return iRc;
		}
		nLen -= iRc;
		lpBuf += iRc;
	}
//	TRACE1( "CSimpleSocket::ReceiveBlk ret=%d\n", nBufLen - nLen );
	return nBufLen - nLen;
}

int CTcpSocket::ReceiveHeader( CArray<CStringA>& arrLines, LPCSTR pszVersion /* = NULL */)
{
	int nLen = 8192;
	CStringA str;
	LPSTR lpBuf = str.GetBuffer( nLen );
	LPSTR lpB = lpBuf;
	int iRc = 0;
	int nRead = 0;
	int nVerLen = 0;
	BOOL bVerified = TRUE;
	if ( pszVersion != NULL )
	{
		bVerified = FALSE;
		nVerLen = (int)strlen( pszVersion );
	}

	while ( nLen > 0 )
	{  
		iRc = Receive( lpB, nLen, 0 );
		if ( iRc == SOCKET_ERROR || iRc == 0 )
			break;
			
		nLen -= iRc;
		nRead += iRc;
		lpB += iRc;

		if ( !bVerified && (nRead >= nVerLen) )
		{
			if ( strncmp( lpBuf, pszVersion, nVerLen ) == 0 )
				bVerified = TRUE;
			else
				break;	// no header
		}
		if ( nRead >= 4 )
		{
			if ( strstr( lpBuf, "\r\n\r\n" ) != NULL )
				break;
		}
	}
	str.ReleaseBufferSetLength( nRead );

	int start = 0;
	if ( bVerified )
	{
//		TRACE1( "ReceiveHeader str=%s\n", str );
		int pos = str.Find( "\r\n", start );
		while( pos > start )
		{
			if ( (pos-start) > 0 )
				arrLines.Add( str.Mid( start, pos-start ) );
			start = pos + 2;
			pos = str.Find( "\r\n", start );
		}
		if ( pos == start )
			start += 2;		// skip crlf
	}
	m_strExtra = str.Mid( start );
//	TRACE1( "CSimpleSocket::ReceiveHdr ret=%d\n", iRc );
	return ( iRc );
}

int CTcpSocket::ReceiveBlkX( LPSTR lpBuf, int nBufLen, int nFlags)
{
	// read strExtra before calling receive
	int nExtra = m_strExtra.GetLength();
	int nBufLen2 = nBufLen - nExtra;
	if ( nBufLen2 <= 0 )		// strExtra > buf: return top of strExtry
	{
		memcpy_s( lpBuf, nBufLen, (LPCSTR)m_strExtra, nBufLen );
		m_strExtra = m_strExtra.Mid( nBufLen );
		return nBufLen;
	}
	memcpy_s( lpBuf, nBufLen, (LPCSTR)m_strExtra, nExtra );
	int iRc = ReceiveBlk( lpBuf + nExtra, nBufLen2, nFlags);
	if ( iRc < 1 )
		return iRc;		// error
	m_strExtra.Empty();
//	TRACE1( "CSimpleSocket::ReceiveBlkX ret=%d\n", nExtra + iRc );
	return nExtra + iRc; // full length
}

int CTcpSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	if ( nBufLen == 0 )
		return 0;
	return send(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}

// ################################

CUdpSocket::CUdpSocket()
{
	m_pAddrMulti = NULL;
	m_pAddrBind = NULL;
	m_pAddrIF = NULL;
}

CUdpSocket::~CUdpSocket()
{
	if (m_pAddrMulti != NULL)
		freeaddrinfo( m_pAddrMulti );
	if (m_pAddrBind != NULL)
		freeaddrinfo( m_pAddrBind );
	if (m_pAddrIF != NULL)
		freeaddrinfo( m_pAddrIF );
}

BOOL CUdpSocket::Create()
{
	ASSERT(m_hSocket == INVALID_SOCKET);

	m_hSocket = socket( m_pAddrMulti->ai_family, m_pAddrMulti->ai_socktype, m_pAddrMulti->ai_protocol );
	if (m_hSocket == INVALID_SOCKET)
		return FALSE;

	int rc = bind( m_hSocket, m_pAddrBind->ai_addr, (int)m_pAddrBind->ai_addrlen);
	if ( rc == SOCKET_ERROR )
	{
        TRACE1("CUdpSocket::Create() bind failed with error code %d\n", WSAGetLastError());
		// int nResult = GetLastError();
		Close();
		// WSASetLastError(nResult);

		return FALSE;
	}
	// set timeout
    rc = setsockopt (m_hSocket, SOL_SOCKET, SO_RCVTIMEO,
				(const char*)&m_nMilliSecRcvTimeOut, 
				sizeof(m_nMilliSecRcvTimeOut) );
	if ( rc == SOCKET_ERROR )
	{
        TRACE1("CUdpSocket::Create() setsockopt failed with error code %d\n", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL CUdpSocket::Create( const char *addr, const char *port, int af, int type, int proto )
{
	int rc = ResolveAddress( Multi, addr, port, af, type, proto );
	if ( rc != 0 )
		return FALSE;

	ASSERT(m_hSocket == INVALID_SOCKET);

	m_hSocket = socket( m_pAddrMulti->ai_family, m_pAddrMulti->ai_socktype, m_pAddrMulti->ai_protocol );
	return (m_hSocket != INVALID_SOCKET);
}

int CUdpSocket::ReceiveFrom(void* lpBuf, int nBufLen, int nFlags, SOCKADDR *safrom, int *fromlen)
{
	int iRc = recvfrom(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags, safrom, fromlen);
#ifdef _DEBUG
    if (iRc == SOCKET_ERROR)
    {
//        TRACE1("CUdpSocket::ReceiveFrom() failed with error code %d\n", WSAGetLastError());
    }
	else
	{
		char buf[256] = {0};
        FormatAddress(safrom, *fromlen, buf, sizeof(buf));
//		TRACE2( "CUdpSocket::ReceiveFrom ret=%d from <%s>\n", iRc, buf );
	}
#endif
	return iRc;
}


int CUdpSocket::SendTo(const void* lpBuf, int nBufLen, int nFlags)
{
	if ( nBufLen == 0 )
		return 0;
//	TRACE1( "CUdpSocket::SendTo n=%d\n", nBufLen );
	return sendto(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags, m_pAddrMulti->ai_addr, (int)m_pAddrMulti->ai_addrlen);
}

// Function: JoinMulticastGroup
// Description:
//    This function joins the multicast socket on the specified multicast
//    group. The structures for IPv4 and IPv6 multicast joins are slightly
//    different which requires different handlers. For IPv6 the scope-ID
//    (interface index) is specified for the local interface whereas for IPv4
//    the actual IPv4 address of the interface is given.
BOOL CUdpSocket::JoinMulticastGroup()
{
    struct ip_mreq   mreqv4;
    struct ipv6_mreq mreqv6;
    char *optval=NULL;
    int    optlevel, option, optlen, rc;
 
    rc = NO_ERROR;
    if (m_pAddrMulti->ai_family == AF_INET)
    {
        // Setup the v4 option values and ip_mreq structure
        optlevel = IPPROTO_IP;
        option   = IP_ADD_MEMBERSHIP;
        optval   = (char *)& mreqv4;
        optlen   = sizeof(mreqv4);
 
        mreqv4.imr_multiaddr.s_addr = ((SOCKADDR_IN *)m_pAddrMulti->ai_addr)->sin_addr.s_addr;
        mreqv4.imr_interface.s_addr = ((SOCKADDR_IN *)m_pAddrIF->ai_addr)->sin_addr.s_addr;
    }
    else if (m_pAddrMulti->ai_family == AF_INET6)
    {
        // Setup the v6 option values and ipv6_mreq structure
        optlevel = IPPROTO_IPV6;
        option   = IPV6_ADD_MEMBERSHIP;
        optval   = (char *) &mreqv6;
        optlen   = sizeof(mreqv6);
 
        mreqv6.ipv6mr_multiaddr = ((SOCKADDR_IN6 *)m_pAddrMulti->ai_addr)->sin6_addr;
        mreqv6.ipv6mr_interface = ((SOCKADDR_IN6 *)m_pAddrIF->ai_addr)->sin6_scope_id;
    }
    else
    {
        TRACE0("Attempting to join multicast group for invalid address family!\n");
        rc = SOCKET_ERROR;
    }
    if (rc != SOCKET_ERROR)
    {
        // Join the group
        rc = setsockopt(m_hSocket, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            TRACE1("JoinMulticastGroup: setsockopt failed with error code %d\n", WSAGetLastError());
        }
#ifdef _DEBUG
        else
        {
			char buf[256] = {0};
            FormatAddress(m_pAddrMulti->ai_addr, m_pAddrMulti->ai_addrlen, buf, sizeof(buf));
            TRACE1("Joined group: %s\n", buf);
        }
#endif
    }
    return ( rc != SOCKET_ERROR);
}
 
// Function: SetSendInterface
// Description:
//    This routine sets the send (outgoing) interface of the socket.
//    Again, for v4 the IP address is used to specify the interface while
//    for v6 its the scope-ID.
BOOL CUdpSocket::SetSendInterface()
{
    char *optval=NULL;
    int   optlevel, option, optlen, rc;
 
    rc = NO_ERROR;
 
    if (m_pAddrIF->ai_family == AF_INET)
    {
        // Setup the v4 option values
        optlevel = IPPROTO_IP;
        option   = IP_MULTICAST_IF;
        optval   = (char *) &((SOCKADDR_IN *)m_pAddrIF->ai_addr)->sin_addr.s_addr;
        optlen   = sizeof(((SOCKADDR_IN *)m_pAddrIF->ai_addr)->sin_addr.s_addr);
    }
    else if (m_pAddrIF->ai_family == AF_INET6)
    {
        // Setup the v6 option values
        optlevel = IPPROTO_IPV6;
        option   = IPV6_MULTICAST_IF;
        optval   = (char *) &((SOCKADDR_IN6 *)m_pAddrIF->ai_addr)->sin6_scope_id;
        optlen   = sizeof(((SOCKADDR_IN6 *)m_pAddrIF->ai_addr)->sin6_scope_id);
    }
    else
    {
        TRACE0("Attempting to set sent interface for invalid address family!\n");
        rc = SOCKET_ERROR;
    }
 
    // Set send IF
    if (rc != SOCKET_ERROR)
    {
        // Set the send interface
        rc = setsockopt(m_hSocket, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            TRACE1("setsockopt() failed with error code %d\n", WSAGetLastError());
        }
#ifdef _DEBUG
        else
        {
			char buf[256] = {0};
            FormatAddress(m_pAddrIF->ai_addr, m_pAddrIF->ai_addrlen, buf, sizeof(buf));
            TRACE1("Set sending interface to: %s\n", buf);
        }
#endif
    }
    return ( rc != SOCKET_ERROR);
}
 
// Function: FormatAddress
// Description:
//    This is similar to the PrintAddress function except that instead of
//    printing the string address to the console, it is formatted into
//    the supplied string buffer.
int CUdpSocket::FormatAddress(SOCKADDR *sa, int salen, char *addrbuf, int addrbuflen)
{
    char    host[NI_MAXHOST], serv[NI_MAXSERV];
    int     hostlen = NI_MAXHOST, servlen = NI_MAXSERV, rc;
 
    rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
    if (rc != 0)
    {
		TRACE2("%s: getnameinfo failed: %d\n", __FILE__, rc);
        return rc;
    }
    if ( (strlen(host) + strlen(serv) + 3) > (unsigned)addrbuflen)
        return WSAEFAULT;
    if (sa->sa_family == AF_INET)
        sprintf_s(addrbuf, addrbuflen, "%s:%s", host, serv);
    else if (sa->sa_family == AF_INET6)
        sprintf_s(addrbuf, addrbuflen, "[%s]:%s", host, serv);
    else
        addrbuf[0] = '\0';
 
    return NO_ERROR;
}

// Function: ResolveAddress
// Description:
//    This routine resolves the specified address and returns a list of addrinfo
//    structure containing SOCKADDR structures representing the resolved addresses.
//    Note that if 'addr' is non-NULL, then getaddrinfo will resolve it whether
//    it is a string literal address or a hostname.
//	returns 0 if ok
int CUdpSocket::ResolveAddress(enum AddrType addrType, const char *addr, const char *port, int af, int type, int proto)
{
    struct addrinfo hints, *res = NULL;
    int             rc;
 
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = af;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;
 
    rc = getaddrinfo(addr, port, &hints, &res);
    if (rc != 0)
    {
		res = NULL;
        TRACE2("Invalid address %s, getaddrinfo failed: %d\n", CString(addr), rc);
    }
	switch (addrType) {
	case Multi:
		ASSERT(m_pAddrMulti == NULL);
		m_pAddrMulti = res;
		break;
	case Bind:
		ASSERT(m_pAddrBind == NULL);
		m_pAddrBind = res;
		break;
	case IF:
		ASSERT(m_pAddrIF == NULL);
		m_pAddrIF = res;
		break;
	}
#ifdef _DEBUG
    TRACE1("CUdpSocket::ResolveAddress %s\n", CString(addr));
	while (res != NULL)
	{
		char buf[256] = {0};
        FormatAddress(res->ai_addr, res->ai_addrlen, buf, sizeof(buf));
        TRACE1(" found: %s\n", CString(buf));
		res = res->ai_next;
	}
#endif
//	freeaddrinfo(res);		see destructor
    return rc;
}

#include "StdAfx.h"
#include "Ws2tcpip.h"
#include "Ping.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


#define SOCKETS_STARTUP_VERSION MAKEWORD( 2, 0 )
int CPing::g_nInit = 0;

CPing::CPing(void)
{
	m_ipaddr = INADDR_NONE;
	Init();
}


CPing::~CPing(void)
{
	Exit();
}

BOOL CPing::Init()
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

BOOL CPing::Exit()
{
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

BOOL CPing::SetHostIP( const char *pszHostIP )
{
	int iR = InetPtonA(AF_INET, pszHostIP, &m_ipaddr);
	if (iR == 1)
		return TRUE;
	if (iR < 0)
		return FALSE;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	iR = getaddrinfo(pszHostIP, NULL, &hints, &result);
	if (iR != 0 || result == NULL)
		return FALSE;
	struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *)result->ai_addr;
	m_ipaddr = sockaddr_ipv4->sin_addr.S_un.S_addr;
	freeaddrinfo(result);
	/*
    m_ipaddr = inet_addr(pszHostIP);
    if (m_ipaddr == INADDR_NONE) {
		// ASSERT(g_nInit > 0);
		LPHOSTENT pHost = gethostbyname( pszHostIP );
		if (pHost == NULL || pHost->h_addrtype != AF_INET || pHost->h_addr_list[0] == 0) {
			// TRACE1("bad IP or unknown host: %s\n", pszHostIP);
			return FALSE;
		}
		m_ipaddr = *(u_long *) pHost->h_addr_list[0];
    }
	*/
	return TRUE;
}

BOOL CPing::SendEcho()
{
    HANDLE hIcmpFile;
    DWORD dwRetVal = 0;
    char szSendData[32] = "CPing Data Buffer";
    LPVOID pReplyBuffer = NULL;
    DWORD dwReplySize = 0;

	hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        // TRACE0("\tUnable to open handle.\n");
        // TRACE1("IcmpCreatefile returned error: %ld\n", GetLastError() );
        return FALSE;
    }    

    dwReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(szSendData);
    pReplyBuffer = (VOID*) malloc(dwReplySize);
    if (pReplyBuffer == NULL) {
        // TRACE0("\tUnable to allocate memory\n");
		IcmpCloseHandle(hIcmpFile);
        return FALSE;
    }    
    
	BOOL bRet = TRUE;
    dwRetVal = IcmpSendEcho(hIcmpFile, m_ipaddr, szSendData, sizeof(szSendData), 
        NULL, pReplyBuffer, dwReplySize, 1000);
    if (dwRetVal != 0) {
    //    PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)pReplyBuffer;
    //    struct in_addr iaReply;
    //    iaReply.S_un.S_addr = pEchoReply->Address;
    //    printf("\tSent icmp message to %s\n", argv[1]);
    //    if (dwRetVal > 1) {
    //        printf("\tReceived %ld icmp message responses\n", dwRetVal);
    //        printf("\tInformation from the first response:\n"); 
    //    }    
    //    else {    
    //        printf("\tReceived %ld icmp message response\n", dwRetVal);
    //        printf("\tInformation from this response:\n"); 
    //    }    
    //    printf("\t  Received from %s\n", inet_ntoa( iaReply ) );
    //    printf("\t  Status = %ld\n", pEchoReply->Status);
    //    printf("\t  Roundtrip time = %ld milliseconds\n", pEchoReply->RoundTripTime);
    }
    else {
    //    printf("\tCall to IcmpSendEcho failed.\n");
    //    printf("\tIcmpSendEcho returned error: %ld\n", GetLastError() );
        bRet = FALSE;
    }
	free( pReplyBuffer );
	IcmpCloseHandle(hIcmpFile);
	return bRet;
}

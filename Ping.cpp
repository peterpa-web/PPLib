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

    dwReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(szSendData) + 8;
    pReplyBuffer = (VOID*) malloc(dwReplySize);
    if (pReplyBuffer == NULL) {
        // TRACE0("\tUnable to allocate memory\n");
		IcmpCloseHandle(hIcmpFile);
        return FALSE;
    }    
    
	BOOL bRet = FALSE;
    TRACE("Send icmp message to %xh (%d.%d.%d.%d)\n", m_ipaddr, m_ipaddr & 0xff, 
		(m_ipaddr >> 8) & 0xff, (m_ipaddr >> 16) & 0xff, (m_ipaddr >> 24) & 0xff);
	dwRetVal = IcmpSendEcho(hIcmpFile, m_ipaddr, szSendData, sizeof(szSendData),
        NULL, pReplyBuffer, dwReplySize, 1000);
    if (dwRetVal != 0) {
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)pReplyBuffer;
#ifdef DEBUG
//        struct in_addr iaReply;
//        iaReply.S_un.S_addr = pEchoReply->Address;
        if (dwRetVal > 1) {
			TRACE1("Received %d icmp message responses\n", dwRetVal);
			TRACE0("Information from the first response:\n");
        }    
        else {    
			TRACE1("Received %d icmp message response\n", dwRetVal);
			TRACE0("Information from this response:\n");
        }    
//		TRACE1("  Received from %S\n", inet_ntoa( iaReply ) );
		TRACE1("  Status = %d\n", pEchoReply->Status);
		TRACE1("  Roundtrip time = %d milliseconds\n", pEchoReply->RoundTripTime);
#endif // DEBUG
		if (pEchoReply->Status == IP_SUCCESS)
			bRet = TRUE;
    }
    else {
    //    printf("\tCall to IcmpSendEcho failed.\n");
        TRACE1("IcmpSendEcho returned error: %ld\n", GetLastError() );
    }
	free( pReplyBuffer );
	IcmpCloseHandle(hIcmpFile);
	return bRet;
}

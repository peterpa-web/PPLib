#pragma once
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
	typedef int BOOL;
#endif

class CPing
{

public:
	CPing(void);
	~CPing(void);

	BOOL SetHostIP( const char *pszHostIP );
	BOOL SendEcho();

protected:
	BOOL Init();
	BOOL Exit();

	static int g_nInit;
	unsigned long m_ipaddr;
};


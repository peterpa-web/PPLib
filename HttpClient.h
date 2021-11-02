#pragma once
#include "winhttp.h"

class CHttpSession
{
public:
	CHttpSession(LPCWSTR pszAgent);
	~CHttpSession() { if (m_hSession) WinHttpCloseHandle(m_hSession); }
	HINTERNET GetHandle() { return m_hSession; }

protected:
	HINTERNET m_hSession = NULL;
};

class CHttpConnection
{
public:
	CHttpConnection(CHttpSession& s, const CString& strServer, INTERNET_PORT nPort);
	~CHttpConnection() { if (m_hConn) WinHttpCloseHandle(m_hConn); }
	HINTERNET GetHandle() { return m_hConn; }

protected:
	CHttpSession& m_session;
	CString m_strServer;
	INTERNET_PORT m_nPort;
	HINTERNET m_hConn = NULL;
};

class CHttpRequest
{
public:
	CHttpRequest(CHttpConnection& c, const CString& strVerb, const CString& strPath);
	~CHttpRequest() { if (m_hRequest) WinHttpCloseHandle(m_hRequest); }
	DWORD SendReceive(CStringA& strRsp);

protected:
	CHttpConnection& m_conn;
	CString m_strVerb;
	CString m_strPath;
	HINTERNET m_hRequest = NULL;
};

class CHttpClient
{
public:
	CHttpClient(LPCWSTR pszAgent) : m_strAgent(pszAgent) {}
	void SetUrl(const CString& strUrl);
	DWORD Get(CStringA& strRsp);

protected:
	CString m_strAgent;
	CString m_strServer;
	INTERNET_PORT m_nPort;
	CString m_strPath;
};


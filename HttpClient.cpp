#include "stdafx.h"
#include "InetException.h"

#include "HttpClient.h"

#pragma comment( lib,"winhttp" )

CHttpSession::CHttpSession(LPCWSTR pszAgent)
{
	m_hSession = WinHttpOpen(pszAgent, WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (m_hSession == NULL)
		throw new CInetException(L"WinHttpOpen", GetLastError());
}

CHttpConnection::CHttpConnection(CHttpSession& s, const CString& strServer, INTERNET_PORT nPort) :
	m_session(s), m_strServer(strServer), m_nPort(nPort)
{
	m_hConn = WinHttpConnect(s.GetHandle(), strServer, nPort, 0);
	if (m_hConn == NULL)
		throw new CInetException(L"WinHttpConnect", GetLastError());
}

CHttpRequest::CHttpRequest(CHttpConnection& c, const CString& strVerb, const CString& strPath) : 
	m_conn(c), m_strVerb(strVerb), m_strPath(strPath)
{
	m_hRequest = WinHttpOpenRequest(c.GetHandle(), strVerb, strPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (m_hRequest == NULL)
		throw new CInetException(L"WinHttpOpenRequest", GetLastError());
}

CStringA CHttpRequest::SendReceive()
{
	CStringA strRsp;
	ASSERT(m_hRequest);
	BOOL bRes = WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	if (!bRes)
		throw new CInetException(L"WinHttpSendRequest", GetLastError());

	bRes = WinHttpReceiveResponse(m_hRequest, NULL);
	if (!bRes)
		throw new CInetException(L"WinHttpReceiveResponse", GetLastError());

	DWORD dwSize = 0;
	if (bRes) do
	{
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(m_hRequest, &dwSize))
			throw new CInetException(L"WinHttpQueryDataAvailable", GetLastError());
		if (dwSize == 0)
			break;

		CStringA strRspPart;
		LPSTR pBuf = strRspPart.GetBuffer(dwSize + 1);
		DWORD dwRead = 0;
		if (!WinHttpReadData(m_hRequest, pBuf, dwSize, &dwRead))
			throw new CInetException(L"WinHttpReadData", GetLastError());

		strRspPart.ReleaseBuffer(dwRead);
		strRsp += strRspPart;
	} while (dwSize > 0);
	return strRsp;
}

void CHttpClient::SetUrl(const CString& strUrl)
{
	URL_COMPONENTS urlComp;
	urlComp.dwStructSize = sizeof(urlComp);
	urlComp.lpszScheme = NULL;
	urlComp.dwSchemeLength = 0;
	urlComp.lpszHostName = NULL;
	urlComp.dwHostNameLength = 1;
	urlComp.lpszUserName = NULL;
	urlComp.dwUserNameLength = 0;
	urlComp.lpszPassword = NULL;
	urlComp.dwPasswordLength = 0;
	urlComp.lpszUrlPath = NULL;
	urlComp.dwUrlPathLength = 1;
	urlComp.lpszExtraInfo = NULL;
	urlComp.dwExtraInfoLength = 0;

	if (!WinHttpCrackUrl(strUrl, strUrl.GetLength(), 0, &urlComp))
		throw new CInetException(L"WinHttpCrackUrl", GetLastError());

	if (urlComp.dwHostNameLength > 0)
		m_strServer = CString(urlComp.lpszHostName, urlComp.dwHostNameLength);
	m_nPort = urlComp.nPort;
	if (urlComp.dwUrlPathLength > 0)
		m_strPath = CString(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
}

CStringA CHttpClient::Get()
{
	CHttpSession session(m_strAgent);
	CHttpConnection conn(session, m_strServer, m_nPort);
	CHttpRequest requ(conn, L"GET", m_strPath);
	return requ.SendReceive();
}

// https://stackoverflow.com/questions/23906654/how-to-get-http-status-code-from-winhttp-request

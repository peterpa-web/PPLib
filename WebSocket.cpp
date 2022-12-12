#include "stdafx.h"
#include "ThreadSocket.h"
#include "Wincrypt.h"
#include "WebSocket.h"

#pragma comment( lib,"Crypt32.lib" )
#define SAH1LEN  20

CWebSocket::~CWebSocket()
{
	Close();
}

BOOL CWebSocket::Init(const CStringA &strKey)
{
	// sample key: dGhlIHNhbXBsZSBub25jZQ==
	// The server would then take the SHA-1 hash of this
	// string, giving the value 0xb3 0x7a 0x4f 0x2c 0xc0 0x62 0x4f 0x16 0x90
	//	0xf6 0x46 0x06 0xcf 0x38 0x59 0x45 0xb2 0xbe 0xc4 0xea.  This value
	//	is then base64 - encoded, to give the value
	//	"s3pPLMBiTxaQ9kYGzzhZRbK+xOo="

	CStringA strRawKey = strKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	BYTE rgbHash[SAH1LEN];
	DWORD dwRc = HashSHA1(strRawKey, rgbHash);
	if (dwRc != 0) {
		TRACE1("(%d) CWebSocket::Init HashSHA1 failed: %d\n", dwRc);
		return FALSE;
	}
	CStringA strRspKey = Base64(rgbHash);
	CStringA strRsp;
	strRsp.Format("HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: %s\r\n\r\n", (LPCSTR)strRspKey);
	m_pSocket->Send(strRsp);
	return TRUE;
}

// https://social.msdn.microsoft.com/Forums/vstudio/en-US/c43c3b36-c698-45da-89f4-ecd0e825c059/implementing-sha1-hash-using-windows-cryptography-api-and-c?forum=vcgeneral
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa382380.aspx

DWORD CWebSocket::HashSHA1(const CStringA &strKey, BYTE *rgbHash)
{
	DWORD dwStatus = 0;		// expected return value

	DWORD cbKey = strKey.GetLength();
	const BYTE *rgbKey = (const BYTE *)(LPCSTR)strKey;

	BOOL bResult = FALSE;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	DWORD cbHash = 0;

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		return dwStatus;
	}

	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		CryptReleaseContext(hProv, 0);
		return dwStatus;
	}

	if (!CryptHashData(hHash, rgbKey, cbKey, 0))
	{
		dwStatus = GetLastError();
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		return dwStatus;
	}

	cbHash = SAH1LEN;
	if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
	{
		dwStatus = GetLastError();
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return dwStatus;
}

CStringA CWebSocket::Base64(const BYTE *rgbHash)
{
	CString str;
	DWORD dwLen = 0;
	VERIFY(CryptBinaryToString(rgbHash, SAH1LEN, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &dwLen));
	LPTSTR pBuf = str.GetBuffer(dwLen);
	VERIFY(CryptBinaryToString(rgbHash, SAH1LEN, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, pBuf, &dwLen));
	str.ReleaseBuffer();
	return CStringA(str);
}

void CWebSocket::RecLoop(CWnd *pWndTarget, UINT nMsg)
{
	pWndTarget->PostMessage( nMsg, init, (LPARAM) this );
	VERIFY( m_pSocket->SetReceiveTimeout(0) );
	while (TRUE) {
		// rec. hdr
		BYTE h[2];
		BYTE m[4];
		int iRc = m_pSocket->ReceiveBlk((LPSTR)h, 2, 0);
		if (iRc == SOCKET_ERROR || iRc == 0)
		{
			CString str = m_pSocket->GetLastError(iRc, 2);
			TRACE1("CWebSocket::RecLoop hdr %s\n", str);
			Close();
			break;
		}

		bool bFin = (h[0] & 0x80) == 0x80;
		int nCmd = h[0] & 7;	// 0=cont 1=text 2=bin 8=close 9=ping A=pong
		bool bMask = (h[1] & 0x80) == 0x80;
		int nLen = h[1] & 0x7f;
		if (nLen > 125) {
			TRACE1("CWebSocket::RecLoop not impl nLen=%d > 125\n", nLen);
			break;
		}
		if (bMask) {
			iRc = m_pSocket->ReceiveBlk((LPSTR)m, 4, 0);
			if (iRc == SOCKET_ERROR || iRc == 0)
			{
                CString str = m_pSocket->GetLastError(iRc, 4);
                TRACE1("CWebSocket::RecLoop mask %s\n", str);
				m_pSocket->Close();
				break;
			}
		}
		// rec. data
		CStringA strBuf;
		LPSTR lpBuf = strBuf.GetBuffer(nLen);
		iRc = m_pSocket->ReceiveBlk(lpBuf, nLen, 0);
		if (iRc == SOCKET_ERROR || iRc == 0)
		{
			CString str = m_pSocket->GetLastError(iRc, nLen);
			TRACE1("CWebSocket::RecLoop data %s\n", str);
			Close();
			break;
        }
		if (bMask) {
			BYTE* lpB = (BYTE*)lpBuf;
            for (int c = 0; c < nLen; c++) {
                lpB[c] ^= m[c % 4];
            }
        }
        strBuf.ReleaseBuffer(nLen);
		if (nCmd == 1) {	// text
			CA2T szr(strBuf, CP_UTF8);
//			TRACE1("CWebSocket::RecLoop text=%s\n", szr);
			pWndTarget->PostMessage( nMsg, text, (LPARAM) new CString( szr ) );
		}
		else {
			TRACE1("CWebSocket::RecLoop not impl nCmd=%d\n", nCmd);
			break;
		}
	}
	if (m_pSocket != nullptr && !m_pSocket->IsClosed())
		pWndTarget->PostMessage( nMsg, exit, NULL );
	TRACE0("CWebSocket::RecLoop exit\n");
}

BOOL CWebSocket::SendShortText(const CString &strMsg)
{
	const CStringA utf8Msg = CW2A(strMsg, CP_UTF8);
	int nLen = utf8Msg.GetLength();
	if (nLen > 125)
		return FALSE;
	BYTE h[2];
	h[0] = 0x81;
	h[1] = nLen;
	nLen += 2;
	CStringA strMsgRaw((char *)h, 2);
	strMsgRaw += utf8Msg;
	if (m_pSocket == nullptr)
		return FALSE;
	int nSent = m_pSocket->Send((LPCSTR)strMsgRaw, nLen, 0);
	return nSent == nLen;
}

void CWebSocket::Close()
{
	if (m_pSocket != nullptr)
		m_pSocket->Close();
	m_pSocket = nullptr;
}


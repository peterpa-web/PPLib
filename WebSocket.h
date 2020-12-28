#pragma once
class CThreadSocket;

class CWebSocket
{
public:
	CWebSocket(CThreadSocket *pSocket) { m_pSocket = pSocket; }
	~CWebSocket();

	BOOL Init(const CStringA &strKey);
	void RecLoop(CWnd *pWndTarget, UINT nMsg);
	BOOL SendShortText(const CString &strMsg);
	void Close();

	enum M {
		init = 0,
		text = 1,
		exit = 9
	};

protected:
	CThreadSocket *m_pSocket;

	DWORD HashSHA1(const CStringA &strKey, BYTE *rgbHash);
	CStringA Base64(const BYTE *rgbHash);
};


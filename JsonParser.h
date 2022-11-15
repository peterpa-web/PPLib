#pragma once
#include "afxmt.h"

class CJsonParser
{
public:
	~CJsonParser();
	bool Parse(CStringA strInput);

protected:
	virtual void StorePair(const CStringA& strContext, const CStringA& strKey, const CStringA& strValue) {}
	virtual void StoreValue(const CStringA& strContext, int nIndex, const CStringA& strValue) {}
	bool PrepThread();
	bool ContThread(const CStringA& strNextInput);
	bool FinThread();		// wait for thread result

private:
	bool WaitThread(int nStart, LPCTSTR pszMsg);	// wait for ContThread or EOD-Msg

	CStringA m_strData;
	int m_nStart = 0;	// index inside m_strData
	int m_nDone = 0;	// size of already processed data
	CWinThread* m_pThread = nullptr;
	DWORD m_dwWaitEvent = INFINITE;
	CEvent m_evNewData;	// fired by ContThread
	CEvent m_evEmpty;	// fired by WaitThread
	bool m_bCanceled = false;

	UINT static __cdecl Run(LPVOID pParam);
	UINT RunThis();

	bool ParseObject(CStringA strContext);
	bool ParseArray(CStringA strContext);
	bool ParsePair(CStringA strContext);
	bool ParseString(CStringA& strResult);
	bool ParseValue(CStringA& strValue);
	void SkipSpaces();
};


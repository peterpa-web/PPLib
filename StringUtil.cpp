#include "stdafx.h"
#include "EventLog.h"

#include "StringUtil.h"

CStringW CStringUtil::FromUtf8(LPCSTR pszUtf8)
{
	size_t nLen = strlen(pszUtf8);
	if (nLen == 0)
		return CStringW();

	CStringW str;
	LPWSTR pBuf = str.GetBuffer(nLen + 1);
	int nNewLen = MultiByteToWideChar(CP_UTF8, 0, pszUtf8, nLen, pBuf, nLen + 1);
	str.ReleaseBuffer(nNewLen);
	if (nNewLen == 0)
		throw new CEventLogException(L"CStringUtil::FromUtf8 failed");
	return str;
}


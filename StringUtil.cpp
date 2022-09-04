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

BOOL CStringUtil::ReadStringA(CStdioFile& f, CStringA& rString)
{
	rString = "";    // empty string without deallocating
	const int nMaxSize = 128;
	LPSTR lpsz = rString.GetBuffer(nMaxSize);
	LPSTR lpszResult;
	int nLen = 0;
	for (;;)
	{
		lpszResult = fgets(lpsz, nMaxSize + 1, f.m_pStream);
		rString.ReleaseBuffer();

		// handle error/eof case
		if (lpszResult == NULL && !feof(f.m_pStream))
		{
			Afx_clearerr_s(f.m_pStream);
			AfxThrowFileException(CFileException::genericException, _doserrno,
				f.GetFileName());
		}

		// if string is read completely or EOF
		if (lpszResult == NULL ||
			(nLen = AtlStrLen(lpsz)) < nMaxSize ||
			lpsz[nLen - 1] == '\n')
			break;

		nLen = rString.GetLength();
		lpsz = rString.GetBuffer(nMaxSize + nLen) + nLen;
	}

	// remove '\n' from end of string if present
	lpsz = rString.GetBuffer(0);
	nLen = rString.GetLength();
	if (nLen != 0 && lpsz[nLen - 1] == '\n')
		rString.GetBufferSetLength(nLen - 1);

	return nLen != 0;
}

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

int CStringUtil::CompareGer(LPCWSTR pszA, LPCWSTR pszB)
{
	WCHAR a0 = 0;
	WCHAR a1 = 0;
	WCHAR b0 = 0;
	WCHAR b1 = 0;

	do
	{
		if (a1 != 0) { a0 = a1;	a1 = 0; }
		else a0 = *pszA++;
		if (a0 == L'ä') { a0 = L'a'; a1 = 1; }
		else if (a0 == L'ö') { a0 = L'o'; a1 = 1; }
		else if (a0 == L'ü') { a0 = L'u'; a1 = 1; }
		else if (a0 == L'ß') { a0 = L's'; a1 = L's'; }
		else if (a0 == L'Ä') { a0 = L'A'; a1 = 1; }
		else if (a0 == L'Ö') { a0 = L'O'; a1 = 1; }
		else if (a0 == L'Ü') { a0 = L'U'; a1 = 1; }

		if (b1 != 0) { b0 = b1;	b1 = 0; }
		else b0 = *pszB++;
		if (b0 == L'ä') { b0 = L'a'; b1 = 1; }
		else if (b0 == L'ö') { b0 = L'o'; b1 = 1; }
		else if (b0 == L'ü') { b0 = L'u'; b1 = 1; }
		else if (b0 == L'ß') { b0 = L's'; b1 = L's'; }
		else if (b0 == L'Ä') { b0 = L'A'; b1 = 1; }
		else if (b0 == L'Ö') { b0 = L'O'; b1 = 1; }
		else if (b0 == L'Ü') { b0 = L'U'; b1 = 1; }

		if (a0 > b0)
			return 1;
		if (a0 < b0)
			return -1;
	} while (a0 == b0 && a0 != 0);
	return 0;
}

int CStringUtil::CompareGerNoCase(LPCWSTR pszA, LPCWSTR pszB)
{
	WCHAR a0 = 0;
	WCHAR a1 = 0;
	WCHAR b0 = 0;
	WCHAR b1 = 0;

	do
	{
		if (a1 != 0) { a0 = a1;	a1 = 0; }
		else a0 = *pszA++;
		if (a0 == L'ä') { a0 = L'A'; a1 = 1; }
		else if (a0 == L'ö') { a0 = L'O'; a1 = 1; }
		else if (a0 == L'ü') { a0 = L'U'; a1 = 1; }
		else if (a0 == L'ß') { a0 = L'S'; a1 = L'S'; }
		else if (a0 == L'Ä') { a0 = L'A'; a1 = 1; }
		else if (a0 == L'Ö') { a0 = L'O'; a1 = 1; }
		else if (a0 == L'Ü') { a0 = L'U'; a1 = 1; }

		if (b1 != 0) { b0 = b1;	b1 = 0; }
		else b0 = *pszB++;
		if (b0 == L'ä') { b0 = L'A'; b1 = 1; }
		else if (b0 == L'ö') { b0 = L'O'; b1 = 1; }
		else if (b0 == L'ü') { b0 = L'U'; b1 = 1; }
		else if (b0 == L'ß') { b0 = L'S'; b1 = L'S'; }
		else if (b0 == L'Ä') { b0 = L'A'; b1 = 1; }
		else if (b0 == L'Ö') { b0 = L'O'; b1 = 1; }
		else if (b0 == L'Ü') { b0 = L'U'; b1 = 1; }

		if (a0 >= L'a' && a0 <= L'z')
			a0 += L'A' - L'a';
		if (b0 >= L'a' && b0 <= L'z')
			b0 += L'A' - L'a';

		if (a0 > b0)
			return 1;
		if (a0 < b0)
			return -1;
	} while (a0 == b0 && a0 != 0);
	return 0;
}

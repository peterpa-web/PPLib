#pragma once
class CStringUtil
{
public:
	static CStringW FromUtf8(LPCSTR pszUtf8);
	static CStringA ToUtf8(LPCWSTR psz) { CStringA s = CW2A(psz); return s; }
	static BOOL ReadStringA(CStdioFile& f, CStringA& rString);
};


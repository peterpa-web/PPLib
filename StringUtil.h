#pragma once
class CStringUtil
{
public:
	static CStringW FromUtf8(LPCSTR pszUtf8);
	static BOOL ReadStringA(CStdioFile& f, CStringA& rString);
};


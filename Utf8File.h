#pragma once
#include <afx.h>
#include "StringUtil.h"

class CUtf8File :
    public CStdioFile
{
public:
    BOOL ReadStringRaw(CStringA& str) { return CStringUtil::ReadStringA(*this, str); }
    BOOL ReadString(CStringW& str);
    void WriteStringRaw(LPCSTR psz);
    void WriteString(LPCWSTR psz);
};


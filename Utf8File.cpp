#include "stdafx.h"
#include "Utf8File.h"

BOOL CUtf8File::ReadString(CStringW& str)
{
    CStringA strA;
    if (!CStringUtil::ReadStringA(*this, strA))
        return FALSE;
    str = CStringUtil::FromUtf8(strA);
    return TRUE;
}

void CUtf8File::WriteString(LPCWSTR psz)
{
    CStringA strA = CStringUtil::ToUtf8(psz);
    WriteStringRaw(strA);
}

void CUtf8File::WriteStringRaw(LPCSTR psz)
{
	ASSERT(psz != NULL);
	ASSERT(m_pStream != NULL);

	if (psz == NULL)
	{
		AfxThrowInvalidArgException();
	}

	if (fputs(psz, m_pStream) == _TEOF)
		AfxThrowFileException(CFileException::diskFull, _doserrno, m_strFileName);
}

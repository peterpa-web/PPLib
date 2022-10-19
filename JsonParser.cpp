#include "stdafx.h"
#include "JsonParser.h"

bool CJsonParser::Parse(const CStringA strInput)
{
    m_nStart = 0;
    m_strData = strInput;

    return ParseObject("");
}

bool CJsonParser::ParseObject(CStringA strContext)
{
    int nStart = m_nStart;
    char c = m_strData[nStart];
    if (c != '{')
    {
        TRACE3("ParseObject from %d: Bad start char %c at %d\n", c, nStart, m_nStart);
        return false;
    }
    TRACE2("ParseObject %d %S\n", nStart, strContext);
    do {
        if (++m_nStart > m_strData.GetLength())
        {
            TRACE2("ParseObject from %d: EOD at %d\n", nStart, m_nStart);
            return false;
        }
        SkipSpaces();
        if (!ParsePair(strContext))
            return false;
        SkipSpaces();
        c = m_strData[m_nStart];
    } while (c == ',');
    SkipSpaces();
    if (c != '}')
    {
        TRACE3("ParseObject from %d: Bad end char %C at %d\n", c, nStart, m_nStart);
        return false;
    }
    if (++m_nStart > m_strData.GetLength())
    {
        TRACE2("ParseObject from %d: EOD at %d\n", nStart, m_nStart);
        return false;
    }
    TRACE2("ParseObject from %d: done at %d\n", nStart, m_nStart);
    return true;
}

bool CJsonParser::ParseArray(CStringA strContext)
{
    int nStart = m_nStart;
    char c = m_strData[nStart];
    if (c != '[')
        return false;   // bad start char
    TRACE2("ParseArray %d %S\n", nStart, strContext);
    SkipSpaces();
    int nIndex = 0;
    do {
        CStringA strIndex;
        strIndex.Format("[%d]", nIndex++);
        if (++m_nStart > m_strData.GetLength())
        {
            TRACE2("ParseArray from %d: EOD at %d\n", nStart, m_nStart);
            return false;
        }
        SkipSpaces();
        if (!ParseObject(strContext + strIndex))
            return false;
        SkipSpaces();
        c = m_strData[m_nStart];
    } while (c == ',');
    SkipSpaces();
    if (c != ']')
    {
        TRACE3("ParseArray from %d: Bad end char %C at %d\n", c, nStart, m_nStart);
        return false;
    }
    if (++m_nStart > m_strData.GetLength())
    {
        TRACE2("ParseArray from %d: EOD at %d\n", nStart, m_nStart);
        return false;
    }
    TRACE2("ParseArray from %d: done at %d\n", nStart, m_nStart);
    return true;
}

bool CJsonParser::ParsePair(CStringA strContext)
{
    CStringA strKey;
    CStringA strValue;
    int nStart = m_nStart;
    if (!ParseString(strKey))
        return false;
    if (!strContext.IsEmpty())
        strContext += '.';
    strContext += strKey;
    SkipSpaces();
    char c = m_strData[m_nStart];
    if (c != ':')
    {
        TRACE3("ParsePair from %d: Bad separator char %C at %d\n", c, nStart, m_nStart);
        return false;
    }
    if (++m_nStart > m_strData.GetLength())
    {
        TRACE2("ParsePair from %d: EOD at %d\n", nStart, m_nStart);
        return false;
    }
    SkipSpaces();
    c = m_strData[m_nStart];
    if (c == '{')
        return ParseObject(strContext);
    if (c == '[')
        return ParseArray(strContext);
    if (c == '"')
    {
        if (!ParseString(strValue))
            return false;
    }
    else
    {
        if (!ParseValue(strValue))
            return false;
    }
//    TRACE3("StorePair %d %S %S\n", nStart, strKey, strValue);
    StorePair(strContext, strKey, strValue);
    return true;
}

bool CJsonParser::ParseString(CStringA& strResult)
{
    int nStart = m_nStart;
    char c = m_strData[nStart];
    if (c != '"')
    {
        TRACE3("ParseString from %d: Bad start char %C at %d\n", c, nStart, m_nStart);
        return false;
    }
    int p = nStart;
    do {
        p = m_strData.Find('"', p + 1);
        if (p < 0)
        {
            TRACE2("ParseString from %d: EOD at %d\n", nStart, m_nStart);
            return false;
        }
    } while (m_strData[p - 1] == '\\');
    strResult = m_strData.Mid(nStart + 1, p - nStart - 1);
    m_nStart = p + 1;
    if (m_nStart > m_strData.GetLength())
    {
        TRACE2("ParseString from %d: EOD at %d\n", nStart, m_nStart);
        return false;
    }
    return true;
}

bool CJsonParser::ParseValue(CStringA& strValue)
{
    int nStart = m_nStart;
    char c = m_strData[m_nStart];
    while (c != ',' && c != '}' && c != ' ' && c != '\t' && c != '\r' && c != '\n')
    {
        if (++m_nStart > m_strData.GetLength())
        {
            TRACE2("ParseValue from %d: EOD at %d\n", nStart, m_nStart);
            return false;
        }
        c = m_strData[m_nStart];
    }
    strValue = m_strData.Mid(nStart, m_nStart - nStart);
    return true;
}

void CJsonParser::SkipSpaces()
{
    char c = m_strData[m_nStart];
    while (c == ' ' || c == '\t' || c == '\r' || c == '\n')
    {
        c = m_strData[++m_nStart];
    }
}


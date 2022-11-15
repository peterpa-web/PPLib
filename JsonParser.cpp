#include "stdafx.h"
#include "JsonParser.h"

CJsonParser::~CJsonParser()
{
    if (m_pThread != nullptr)
    {
        m_bCanceled = true;
        FinThread();
    }
}

bool CJsonParser::Parse(const CStringA strInput)
{
    m_nStart = 0;
    m_nDone = 0;
    m_strData = strInput;

    return ParseObject("");
}

bool CJsonParser::PrepThread()
{
    TRACE0("PrepThread\n");
    m_pThread = AfxBeginThread(Run, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    if (m_pThread == nullptr)
        return false;
    m_evEmpty.ResetEvent();
    m_evNewData.ResetEvent();
    m_pThread->m_bAutoDelete = FALSE;
    m_pThread->ResumeThread();
    return true;
}

UINT __cdecl CJsonParser::Run(LPVOID pParam)
{
    CJsonParser* pThis = (CJsonParser*)pParam;
    return pThis->RunThis();
}

UINT CJsonParser::RunThis()
{
    m_nStart = 0;
    m_nDone = 0;
    int nStart = 0;
    m_bCanceled = false;

//    m_evEmpty.SetEvent();
    if (!WaitThread(nStart, _T("Run")))
        return 1;
    bool bRet = ParseObject("");
    if (!bRet)
    {
        m_bCanceled = true;
        m_evEmpty.SetEvent();   // allow ContThread detecting cancelation
    }
    return bRet;
}

bool CJsonParser::WaitThread(int nStart, LPCTSTR pszMsg)
{
    if (m_pThread != nullptr)
    {
//        TRACE1("WaitThread %s begin\n", pszMsg);
        m_evEmpty.SetEvent();
        if (m_bCanceled)
        {
            TRACE0("WaitThread canceled\n");
            return false;
        }
        // wait for ContThread providing new input data
        DWORD dwRes = WaitForSingleObject(m_evNewData.m_hObject, m_dwWaitEvent);
        if (dwRes == WAIT_TIMEOUT)
        {
            TRACE3("%s Wait Timeout from %d: EOD at %d\n", pszMsg, nStart, m_nDone + m_nStart);
            return false;
        }
        if (dwRes == WAIT_FAILED)
        {
            TRACE3("%s Wait failed from %d: EOD at %d\n", pszMsg, nStart, m_nDone + m_nStart);
            TRACE1("  Err=%d\n", GetLastError());
            return false;
        }
 //       TRACE1("WaitThread %s end\n", pszMsg);
        m_evNewData.ResetEvent();
        return true;
    }
    TRACE3("%s from %d: EOD at %d\n", pszMsg, nStart, m_nDone + m_nStart);
    return false;
}

bool CJsonParser::ContThread(const CStringA& strNextInput)
{
    ASSERT(m_pThread != nullptr);
    // wait for WaitThread where all input data are processed
    DWORD dwRes = WaitForSingleObject(m_evEmpty.m_hObject, m_dwWaitEvent);
    if (dwRes == WAIT_TIMEOUT)
    {
        TRACE1("ContThread Timeout at %d\n", m_nDone + m_nStart);
        return false;
    }
    if (dwRes == WAIT_FAILED)
    {
        TRACE1("ContThread failed at %d\n", m_nDone + m_nStart);
        TRACE1("  Err=%d\n", GetLastError());
        return false;
    }
 //   TRACE1("ContThread >%S<\n", strNextInput);
    m_evEmpty.ResetEvent();
    if (m_bCanceled)
    {
        TRACE0("ContThread canceled\n");
        return false;
    }
    m_nDone += m_strData.GetLength();
    m_nStart = 0;
    m_strData = strNextInput;
    SkipSpaces();
    m_evNewData.SetEvent();
    return true;
}

bool CJsonParser::FinThread()
{
    TRACE0("FinThread begin\n");
    DWORD dwRes = WaitForSingleObject(m_pThread->m_hThread, m_dwWaitEvent);
    if (dwRes == WAIT_TIMEOUT)
    {
        TRACE1("FinThread Timeout at %d\n", m_nDone + m_nStart);
        return false;
    }
    if (dwRes == WAIT_FAILED)
    {
        TRACE1("FinThread failed at %d\n", m_nDone + m_nStart);
        TRACE1("  Err=%d\n", GetLastError());
        return false;
    }

    // get return value of the thread (your actual question)
    DWORD dwResult;
    if (!GetExitCodeThread(m_pThread->m_hThread, &dwResult))
        return false;
    delete m_pThread;
    m_pThread = nullptr;
    TRACE1("FinThread end ret=%d\n", dwRes);
    return dwRes == 0;
}

bool CJsonParser::ParseObject(CStringA strContext)
{
    int nStart = m_nStart + m_nDone;
    char c = m_strData[m_nStart];
    if (c != '{')
    {
        TRACE3("ParseObject from %d: Bad start char %C at %d\n", c, nStart, m_nDone + m_nStart);
        return false;
    }
//    TRACE2("ParseObject %d %S\n", nStart, strContext);
    do {
        if (++m_nStart >= m_strData.GetLength())
        {
            if (!WaitThread(nStart, _T("ParseObject_A")))
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
        TRACE3("ParseObject from %d: Bad end char %C at %d\n", c, nStart, m_nDone + m_nStart);
        return false;
    }
    if (nStart == 0)
    {
        TRACE2("ParseObject from %d: Finished at %d\n", nStart, m_nDone + m_nStart);
        return true;
    }

    if (++m_nStart >= m_strData.GetLength())
    {
        if (!WaitThread(nStart, _T("ParseObject_B")))
            return false;
    }
//    TRACE2("ParseObject from %d: done at %d\n", nStart, m_nDone + m_nStart);
    return true;
}

bool CJsonParser::ParseArray(CStringA strContext)
{
    int nStart = m_nDone + m_nStart;
    char c = m_strData[m_nStart];
    if (c != '[')
        return false;   // bad start char
//    TRACE2("ParseArray %d %S\n", nStart, strContext);
    SkipSpaces();
    int nIndex = 0;
    do {
        CStringA strIndex;
        strIndex.Format("[%d]", nIndex);
        if (++m_nStart >= m_strData.GetLength())
        {
            if (!WaitThread(nStart, _T("ParseArray_A")))
                return false;
        }
        SkipSpaces();
        CStringA strValue;
        c = m_strData[m_nStart];
        if (c == '{')
        {
            if (!ParseObject(strContext + strIndex))
                return false;
        }
        else if (c == '[')
        {
            if (!ParseArray(strContext + strIndex))
                return false;
        }
        else if (c == '"')
        {
            if (!ParseString(strValue))
                return false;
            StoreValue(strContext, nIndex, strValue);
        }
        else
        {
            if (!ParseValue(strValue))
                return false;
            StoreValue(strContext, nIndex, strValue);
        }
        ++nIndex;
        SkipSpaces();
        c = m_strData[m_nStart];
    } while (c == ',');
    SkipSpaces();
    if (c != ']')
    {
        TRACE3("ParseArray from %d: Bad end char %C at %d\n", c, nStart, m_nDone + m_nStart);
        return false;
    }
    if (++m_nStart >= m_strData.GetLength())
    {
        if (!WaitThread(nStart, _T("ParseArray_B")))
            return false;
    }
 //   TRACE2("ParseArray from %d: done at %d\n", nStart, m_nDone + m_nStart);
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
        TRACE3("ParsePair from %d: Bad separator char %C at %d\n", c, nStart, m_nDone + m_nStart);
        return false;
    }
    if (++m_nStart > m_strData.GetLength())
    {
        if (!WaitThread(nStart, _T("ParsePair_A")))
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
 //       StoreValue(strContext, -1,  strValue);
    }
    else
    {
        if (!ParseValue(strValue))
            return false;
//        StoreValue(strContext, -1, strValue);
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
        TRACE3("ParseString from %d: Bad start char %C at %d\n", c, nStart, m_nDone + m_nStart);
        return false;
    }
    int p = nStart;
    do {
        p = m_strData.Find('"', p + 1);
        if (p < 0)
        {
            if (!WaitThread(nStart, _T("ParseString_A")))
                return false;
            p = 0;
        }
    } while (m_strData[p - 1] == '\\');
    strResult = m_strData.Mid(nStart + 1, p - nStart - 1);
    m_nStart = p + 1;
    if (m_nStart > m_strData.GetLength())
    {
        if (!WaitThread(nStart, _T("ParseString_B")))
            return false;
    }
    return true;
}

bool CJsonParser::ParseValue(CStringA& strValue)
{
    int nStart = m_nStart;
    char c = m_strData[m_nStart];
    while (c != ',' && c != ']' && c != '}' && c != ' ' && c != '\t' && c != '\r' && c != '\n')
    {
        if (++m_nStart > m_strData.GetLength())
        {
            if (!WaitThread(nStart, _T("ParseValue_A")))
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


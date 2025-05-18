#include "stdafx.h"
#include "ShlObj_core.h"

#include "FileTrace.h"

CString CFileTrace::s_strFileBase;
CStdioFile CFileTrace::s_file;
CTime CFileTrace::s_timeNextFile = 0;

void CFileTrace::Init(LPCTSTR pszDir, LPCTSTR pszFile)
{
	TCHAR szPath[MAX_PATH];
	if (FAILED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
	{
		AfxMessageBox(_T("LOCAL_APPDATA Pfad nicht gefunden."), MB_ICONEXCLAMATION);
		return;
	}
	CString strIniPath(szPath);
	CString strDataDir = strIniPath + '\\' + pszDir;
	CFileStatus rStatus;
	if (!CFile::GetStatus(strDataDir, rStatus))
	{
		if (!CreateDirectory(strDataDir, NULL))
		{
			AfxMessageBox(_T("CreateDir mit Fehler: " + strDataDir), MB_ICONEXCLAMATION);
			return;
		}
	}
	s_strFileBase = strDataDir + '\\' + pszFile;
}

void CFileTrace::Write(
	_In_opt_z_ const char* pszFileName,
	_In_ int nLine,
	_In_z_ LPCWSTR pwszFmt,
	_In_ va_list args)
{
	CTime timeNow = CTime::GetCurrentTime();
	if (s_file.m_hFile != INVALID_HANDLE_VALUE && timeNow >= s_timeNextFile)
		s_file.Close();
	if (s_file.m_hFile == INVALID_HANDLE_VALUE)
	{
		ASSERT(!s_strFileBase.IsEmpty());
		CString strTraceFile = timeNow.Format(_T("%e.txt"));
		strTraceFile.Replace(' ', '0');
		strTraceFile = s_strFileBase + strTraceFile;
		CFileStatus fs;
		if (CFile::GetStatus(strTraceFile, fs) &&
			(fs.m_mtime.GetDay() != timeNow.GetDay() || fs.m_mtime.GetMonth() != timeNow.GetMonth()))
			DeleteFile(strTraceFile);
		CFileException* pe = new CFileException;
		if (s_file.Open(strTraceFile, CFile::shareDenyWrite |
			CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate, pe))
			pe->Delete();
		else
			throw pe;
		s_file.SeekToEnd();
		CString strStart = timeNow.Format(_T("\n======= CreateTracefile %y/%m/%d %H:%M:%S\n"));
		s_file.WriteString(strStart);
		s_timeNextFile = CTime(timeNow.GetYear(), timeNow.GetMonth(), timeNow.GetDay(), 0, 0, 0) + CTimeSpan(1, 0, 0, 0);
	}
	CString strLine = timeNow.Format(_T("%H:%M:%S "));
//	if (s_file.m_hFile != INVALID_HANDLE_VALUE)
//		s_file.WriteString(strLine);

	// Explicitly request the legacy wide format specifiers mode from the CRT,
	// for compatibility with previous versions.  While the CRT supports two
	// modes, the ATL and MFC functions that accept format strings only support
	// legacy mode format strings.
	int cchNeeded = __stdio_common_vswprintf(
		_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS |
		_CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR |
		_CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS,
		NULL, 0, pwszFmt, NULL, args);
	if (cchNeeded < 0)
	{
		return;
	}

	CHeapPtr<wchar_t> wszBuf;
	if (!wszBuf.Allocate(cchNeeded + 1))
	{
		return;
	}

	wszBuf[0] = '\0';

	int const vsnwprintf_result = __stdio_common_vsnwprintf_s(
		_CRT_INTERNAL_LOCAL_PRINTF_OPTIONS |
		_CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS,
		wszBuf, cchNeeded + 1, cchNeeded, pwszFmt, NULL, args);
	if (vsnwprintf_result < 0)
	{
		return;
	}

	wchar_t fileName[_MAX_PATH] = { '\0' };
	if (swprintf_s(fileName, _MAX_PATH, L"%hs", pszFileName) == -1)
	{
		return;
	}

	if (s_file.m_hFile != INVALID_HANDLE_VALUE)
	{
		wchar_t* p = wcsrchr(fileName, '\\');
		if (p != nullptr)
			strLine += (p + 1);
	//		s_file.WriteString(p + 1);
		CString strLineNo;
		strLineNo.Format(L"(%d) : ", nLine);
		strLine += strLineNo;
		strLine += wszBuf;
		s_file.WriteString(strLine);
		s_file.Flush();
	}

#ifdef DEBUG
	_CrtDbgReportW(_CRT_WARN, fileName, nLine, nullptr, L"%ls", static_cast<const wchar_t*>(wszBuf));
#endif // DEBUG
}

void CFileTrace::Exit()
{
	if (s_file.m_hFile != INVALID_HANDLE_VALUE)
		s_file.Close();
}

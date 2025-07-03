#pragma once
// compare to atltrace.h
#include <afxmt.h>

#ifndef FILETRACE
#define FILETRACE CFTraceFileAndLineInfo(__FILE__, __LINE__)
#endif

class CFileTrace
{
public:
	static void Init(LPCTSTR pszDir, LPCTSTR pszFile);
	static void Write(
		_In_opt_z_ const char* pszFileName,
		_In_ int nLine,
		_In_z_ LPCWSTR pwszFmt,
		_In_ va_list args);
	static void Exit();
	static bool IsNoInit() { return s_strFileBase.IsEmpty(); }
	static CString GetFilePath();

protected:
	static CString s_strFileBase;
	static CStdioFile s_file;
	static CTime s_timeNextFile;
	static CCriticalSection s_cs;
};

class CFTraceFileAndLineInfo
{
public:
	CFTraceFileAndLineInfo(
		_In_z_ const char* pszFileName,
		_In_ int nLineNo)
		: m_pszFileName(pszFileName), m_nLineNo(nLineNo)
	{
	}
/*
#pragma warning(push)
#pragma warning(disable : 4793)
	void __cdecl operator()(
		_In_ int dwCategory,
		_In_ UINT nLevel,
		_In_z_ const char* pszFmt,
		...) const
	{
		CPreserveLastError ple;
		va_list ptr; va_start(ptr, pszFmt);
	//	ATL::CTrace::TraceV(m_pszFileName, m_nLineNo, dwCategory, nLevel, pszFmt, ptr);
		va_end(ptr);
	}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4793)
	void __cdecl operator()(
		_In_ int dwCategory,
		_In_ UINT nLevel,
		_In_z_ const wchar_t* pszFmt,
		...) const
	{
		CPreserveLastError ple;
		va_list ptr; va_start(ptr, pszFmt);
	//	ATL::CTrace::TraceV(m_pszFileName, m_nLineNo, dwCategory, nLevel, pszFmt, ptr);
		va_end(ptr);
	}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4793)
	void __cdecl operator()(
		_In_z_ const char* pszFmt,
		...) const
	{
		CPreserveLastError ple;
		va_list ptr; va_start(ptr, pszFmt);
	//	ATL::CTrace::TraceV(m_pszFileName, m_nLineNo, atlTraceGeneral, 0, pszFmt, ptr);
		va_end(ptr);
	}
#pragma warning(pop)
*/

#pragma warning(push)
#pragma warning(disable : 4793)
	void __cdecl operator()(
		_In_z_ const wchar_t* pwszFmt,
		...) const
	{
#ifndef DEBUG
if (CFileTrace::IsNoInit())
	return;
#else
		CPreserveLastError ple;
#endif
		va_list ptr; va_start(ptr, pwszFmt);
	//	ATL::CTrace::TraceV(m_pszFileName, m_nLineNo, atlTraceGeneral, 0, pszFmt, ptr);
		CFileTrace::Write(m_pszFileName, m_nLineNo, pwszFmt, ptr);
		va_end(ptr);
	}
#pragma warning(pop)

private:
	/* unimplemented */
	CFTraceFileAndLineInfo& __cdecl operator=(_In_ const CFTraceFileAndLineInfo& right);

	const char* const m_pszFileName;
	const int m_nLineNo;
};

/*
#pragma warning(push)
#pragma warning(disable : 4793)
inline void __cdecl AtlTrace(_In_z_ _Printf_format_string_ LPCSTR pszFormat, ...)
{
	CPreserveLastError ple;
	va_list ptr;
	va_start(ptr, pszFormat);
	ATL::CTrace::TraceV(NULL, -1, atlTraceGeneral, 0, pszFormat, ptr);
	va_end(ptr);
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4793)
inline void __cdecl AtlTrace(_In_z_ _Printf_format_string_ LPCWSTR pszFormat, ...)
{
	CPreserveLastError ple;
	va_list ptr;
	va_start(ptr, pszFormat);
	ATL::CTrace::TraceV(NULL, -1, atlTraceGeneral, 0, pszFormat, ptr);
	va_end(ptr);
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4793)
inline void __cdecl AtlTrace2(
	_In_ int dwCategory,
	_In_ UINT nLevel,
	_In_z_ _Printf_format_string_ LPCSTR pszFormat, ...)
{
	CPreserveLastError ple;
	va_list ptr;
	va_start(ptr, pszFormat);
	ATL::CTrace::TraceV(NULL, -1, dwCategory, nLevel, pszFormat, ptr);
	va_end(ptr);
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 4793)
inline void __cdecl AtlTrace2(
	_In_ int dwCategory,
	_In_ UINT nLevel,
	_In_z_ _Printf_format_string_ LPCWSTR pszFormat, ...)
{
	CPreserveLastError ple;
	va_list ptr;
	va_start(ptr, pszFormat);
	ATL::CTrace::TraceV(NULL, -1, dwCategory, nLevel, pszFormat, ptr);
	va_end(ptr);
}
#pragma warning(pop)
*/

#define FTRACE0(sz)              FTRACE(_T("%Ts"), _T(sz))
#define FTRACE1(sz, p1)          FTRACE(_T(sz), p1)
#define FTRACE2(sz, p1, p2)      FTRACE(_T(sz), p1, p2)
#define FTRACE3(sz, p1, p2, p3)  FTRACE(_T(sz), p1, p2, p3)
#define FTRACE FILETRACE

#define FTRACEINIT(sDir,sFile) CFileTrace::Init(sDir, sFile); FTRACE0("FTRACEINIT\n")
#define FTRACEEXIT FTRACE0("FTRACEEXIT\n"); CFileTrace::Exit()


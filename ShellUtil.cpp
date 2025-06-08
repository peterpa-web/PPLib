#include "stdafx.h"
#include "ShellUtil.h"

void CShellUtil::Explore(const CString& strPath)
{
	TCHAR szCmd[MAX_PATH] = _T("explorer ");
	_tcsncat_s(szCmd, MAX_PATH, strPath, MAX_PATH - 10);
	Exec(szCmd);
}

void CShellUtil::Exec(const CString& strPath, bool bHidden)
{
	TCHAR szCmd[MAX_PATH] = _T("");
	_tcsncat_s(szCmd, MAX_PATH, strPath, MAX_PATH - 1);

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	if (bHidden)
	{
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	BOOL b = CreateProcess(NULL,  // No module name (use command line). 
		szCmd,			 // Command line. 
		NULL,            // Process handle not inheritable. 
		NULL,            // Thread handle not inheritable. 
		FALSE,           // Set handle inheritance to FALSE. 
		0,               // No creation flags. 
		NULL,            // Use parent's environment block. 
		NULL,            // Use parent's starting directory. 
		&si,             // Pointer to STARTUPINFO structure.
		&pi);            // Pointer to PROCESS_INFORMATION structure.
	if (!b)
	{
		DWORD dwLastErr = GetLastError();
		FTRACE2("Exec %s = %d\n", szCmd, dwLastErr);
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}


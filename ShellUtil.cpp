#include "stdafx.h"
#include "ShellUtil.h"

void CShellUtil::Explore(const CString& strPath)
{
	TCHAR szCmd[MAX_PATH] = _T("explorer ");
	_tcsncat_s(szCmd, MAX_PATH, strPath, MAX_PATH - 10);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// Start the child process. 
	CreateProcess(NULL,  // No module name (use command line). 
		szCmd,			  // Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi);            // Pointer to PROCESS_INFORMATION structure.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}


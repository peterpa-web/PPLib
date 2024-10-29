#pragma once
#include <afxmt.h>

class CBaseThread
{
public:
	CBaseThread(LPVOID pParam);
	virtual ~CBaseThread();

	void Start();
	bool IsCanceled() { return m_bCanceled; }
	bool IsBusy() { return m_pThread != nullptr; }
	bool WaitForEnding();

	virtual void Cancel() { m_bCanceled = true; }
	virtual void Run() = 0;
	virtual void OnException(CException* pe);
	virtual void OnEnding() {}

protected:
	LPVOID m_pParam;
	bool m_bCanceled = false;

	UINT WorkThread();

private:
	CWinThread* m_pThread = nullptr;
	static UINT __cdecl WorkThreadStatic(LPVOID pParam);
};


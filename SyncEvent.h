#pragma once
#include <afxmt.h>

class CSyncEvent
{
public:
	CSyncEvent() {}

	bool Wait(DWORD dwMilliseconds = INFINITE, bool* pbTimeout = nullptr);
	void Cancel() { m_bCanceled = true; m_evRun.SetEvent(); }
	void SetEvent() { m_evRun.SetEvent(); }
	bool IsWaiting() { return m_bWaiting; }

protected:
//	CEvent m_evCancel;
	CEvent m_evRun;
	bool m_bWaiting = false;
	bool m_bCanceled = false;
};


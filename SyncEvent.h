#pragma once
#include <afxmt.h>

class CSyncEvent
{
public:
	CSyncEvent() {}

//	DWORD Wait(DWORD dwMilliseconds = INFINITE);	// returns WAIT_FAILED, WAIT_OBJECT_0 (canceled), WAIT_OBJECT_0 + 1 (ok)
	bool Wait(DWORD dwMilliseconds = INFINITE, bool* pbTimeout = nullptr);
	void Cancel() { m_evCancel.SetEvent(); }
	void SetEvent() { m_evRun.SetEvent(); }
//	CEvent& GetEvent() { return m_evRun; }
	bool IsWaiting() { return m_bWaiting; }

protected:
	CEvent m_evCancel;
	CEvent m_evRun;
	bool m_bWaiting = false;
};


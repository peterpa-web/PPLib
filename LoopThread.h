#pragma once
#include "BaseThread.h"
#include "SyncEvent.h"

class CLoopThread :
    public CBaseThread
{
public:
	CLoopThread(LPVOID pParam);

	void Start();
	virtual void Cancel();
	virtual void RunOnce() = 0;
	bool IsWaiting() { return m_sync.IsWaiting(); }
//	void Continue() { m_sync.SetEvent(); }

protected:
	CSyncEvent m_sync;

	virtual void Run() override;
};


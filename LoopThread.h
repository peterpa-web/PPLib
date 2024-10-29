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

/*
Using:
class CAllRTasks
{
protected:
	class CTasksThread : public CLoopThread
	{
	public:
		CTasksThread(LPVOID pParam);

	protected:
		void RunOnce() override;
		void OnException(CException* pe) override;
	};

	CTasksThread m_thread;
	...
}

CAllRTasks::CTasksThread::CTasksThread(LPVOID pParam) : CLoopThread(pParam)
{
}

void CAllRTasks::CTasksThread::RunOnce()
{
	CAllRTasks* pThis = static_cast<CAllRTasks*>(m_pParam);
	TRACE0("CTasksThread::RunOnce() start\n");
	pThis->WorkThread();
	TRACE0("CTasksThread::RunOnce() end\n");
}

void CAllRTasks::CTasksThread::OnException(CException* pe)
{
	CAllRTasks* pThis = static_cast<CAllRTasks*>(m_pParam);
	TCHAR szMsg[512] = _T("");
	pe->GetErrorMessage(szMsg, 512, NULL);
	CTaskRBase* pTask = pThis->GetCurrentTask();
	pTask->ProtocolError(_T("CTasksThread::Exception"), szMsg);
	pe->Delete();
}


*/
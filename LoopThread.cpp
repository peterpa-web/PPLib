#include "stdafx.h"
#include "LoopThread.h"

CLoopThread::CLoopThread(LPVOID pParam) : CBaseThread(pParam)
{
}

void CLoopThread::Start()
{
	m_sync.SetEvent();
	CBaseThread::Start();
}

void CLoopThread::Cancel()
{
	m_sync.Cancel();
	CBaseThread::Cancel();
}

void CLoopThread::Run()
{
	m_bCanceled = false;
	while (!m_bCanceled)
	{
		bool bTimeout = false;
		if (!m_sync.Wait(10000, &bTimeout))
		{
			TRACE1("CLoopThread::Run canceled timeout=%s\n", bTimeout ? L"true" : L"false");
			CBaseThread::Cancel();
			return;
		}
		RunOnce();
	}
}


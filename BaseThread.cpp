#include "stdafx.h"
#include "BaseThread.h"

CBaseThread::CBaseThread(LPVOID pParam) : m_pParam(pParam)
{
}

CBaseThread::~CBaseThread()
{
	VERIFY(WaitForEnding());
}

void CBaseThread::Start()
{
	if (IsBusy())
		return;
	m_bCanceled = false;
	m_pThread = AfxBeginThread(WorkThreadStatic, this);
	ASSERT(m_pThread);
}

bool CBaseThread::WaitForEnding()
{
	if (IsBusy())
	{
		Cancel();
		for (int n = 0; n < 60; n++)
		{
			Sleep(500);
			if (!IsBusy())
				return true;
		}
		return false;	// timeout
	}
	return true;
}

void CBaseThread::OnException(CException* pe)
{
	m_bCanceled = true;
	pe->ReportError();
	pe->Delete();
}

UINT __cdecl CBaseThread::WorkThreadStatic(LPVOID pParam)
{
	CBaseThread* pThis = static_cast<CBaseThread*>(pParam);
	return pThis->WorkThread();
}

UINT CBaseThread::WorkThread()
{
	try
	{
		Run();
		OnEnding();
	}
	catch (CException* pe)
	{
		OnException(pe);
	}
	m_pThread = nullptr;
	return 0;
}


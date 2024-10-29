#include "stdafx.h"
#include "SyncEvent.h"

bool CSyncEvent::Wait(DWORD dwMilliseconds, bool* pbTimeout)
{
    HANDLE ah[2] = { m_evCancel.m_hObject, m_evRun.m_hObject };
    m_bWaiting = true;
    DWORD dwRc = ::WaitForMultipleObjects(2, ah, FALSE, dwMilliseconds);
    m_bWaiting = false;
    if (pbTimeout != nullptr)
        *pbTimeout = dwRc == WAIT_FAILED;
    return dwRc == (WAIT_OBJECT_0 + 1);
}

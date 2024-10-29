#include "stdafx.h"
#include "SyncEvent.h"

bool CSyncEvent::Wait(DWORD dwMilliseconds, bool* pbTimeout)
{
    m_bWaiting = true;
    DWORD dwRc = ::WaitForSingleObject(m_evRun, dwMilliseconds);
    m_bWaiting = false;
    if (m_bCanceled)
        return false;
    if (pbTimeout != nullptr)
        *pbTimeout = dwRc == WAIT_FAILED;
    return dwRc == (WAIT_OBJECT_0);
}

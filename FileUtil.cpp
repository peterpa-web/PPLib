#include "stdafx.h"
#include "FileUtil.h"

CTime CFileUtil::GetFileModifTime(LPCTSTR lpszFileName)
{
	CTime timeLast((time_t)0L);
	CFileStatus fs;
	if (CFile::GetStatus(lpszFileName, fs))
	{
		timeLast = fs.m_mtime;
	}
	return timeLast;
}

void CFileUtil::AssureBaseDir(const CString& strPath)
{
	int p = strPath.ReverseFind('\\');
	if (p < 0)
		CFileException::ThrowErrno(ENOENT, strPath);
	CString strBasePath = strPath.Left(p);
	CFileStatus fs;
	if ((!CFile::GetStatus(strBasePath, fs)) || (fs.m_attribute & CFile::directory) == 0)
	{
		AssureBaseDir(strBasePath);
		if (!CreateDirectory(strBasePath, NULL))
			CFileException::ThrowErrno(GetLastError(), strPath);
	}
}

char* CFileUtil::s_pCopyBuf = NULL;

void CFileUtil::CopyFileProgress(LPCTSTR lpSource, LPCTSTR lpDest)
{
	TRACE2("CopyFileProgress %s %s\n", lpDest, lpSource);
	m_llProgress += m_llFileProgress;
	m_llFileProgress = 0;
	//    return CopyFileEx( lpSource, lpDest, &Progress, this, &s_fCanceled, 0 );

	if (s_pCopyBuf == NULL)
	{
		s_pCopyBuf = new char[0x10000];
	}
	CString strDestTmp = lpDest;
	strDestTmp += '~';
	try {
		CFile fSource(lpSource, CFile::modeRead | CFile::shareDenyWrite);
		CFile fDest(strDestTmp, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive);

		ULONGLONG nLen = fSource.GetLength();
		ULONGLONG nLeft = nLen;

		while (nLeft > 0)
		{
			UINT nCount = 0x10000;
			if (nLeft < nCount)
				nCount = (UINT)nLeft;
			UINT nRead = fSource.Read(s_pCopyBuf, nCount);
			fDest.Write(s_pCopyBuf, nRead);
			nLeft -= nRead;
			m_llFileProgress = nLen - nLeft;
			if (m_fCanceled)
				CFileException::ThrowErrno(ECANCELED, lpDest);
			Sleep(0);
		}
		fDest.Close();

		if (!MoveFileEx(strDestTmp, lpDest, MOVEFILE_REPLACE_EXISTING))
			CFileException::ThrowErrno(GetLastError(), lpDest);

		CFileStatus statSource;
		fSource.GetStatus(statSource);
		CFileStatus statDest;
		CFile::GetStatus(lpDest, statDest);
		statDest.m_ctime = statSource.m_ctime;
		statDest.m_mtime = statSource.m_mtime;
		statDest.m_attribute = statSource.m_attribute;
		CFile::SetStatus(lpDest, statDest);

	}
	catch (CException* pe)
	{
		ASSERT(FALSE);
		DeleteFile(strDestTmp);
		throw pe;
	}
}

int CFileUtil::GetProgress()
{
	if (m_llSize < 100)
		return 0;

	ULONGLONG llSize = m_llSize / 100;
	ULONGLONG p = (m_llProgress + m_llFileProgress) / llSize;
	return (int)p;
}

int CFileUtil::CountFiles(const CString& strDstPath)
{
	int nFiles = 0;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strDstPath + _T("/*.*"));
	while (bWorking)
	{
		if (m_fCanceled)
			return 0;

		bWorking = finder.FindNextFile();
		if (!bWorking)
		{
			DWORD dwLastErr = GetLastError();
			if (dwLastErr != ERROR_NO_MORE_FILES)
				CFileException::ThrowOsError(dwLastErr, strDstPath);
		}
		if (finder.IsDots() || finder.IsHidden() || finder.IsSystem())
			continue;

		if (finder.IsDirectory())
			nFiles += CountFiles(finder.GetFilePath());
		else
			++nFiles;
	}
	return nFiles;
}

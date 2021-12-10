#pragma once
class CFileUtil
{
public:
	CFileUtil(BOOL &fCanceled) : m_fCanceled(fCanceled) {}

	static void CleanStatic() {
		if (s_pCopyBuf != nullptr)
			delete s_pCopyBuf;
		s_pCopyBuf = nullptr;
	}
	static CTime GetFileModifTime(LPCTSTR lpszFileName);
	static void AssureBaseDir(const CString& strPath);

	void CopyFileProgress(LPCTSTR lpSource, LPCTSTR lpDest);
	void SetSize(ULONGLONG llSize) { m_llSize = llSize; }
	void AddSize(ULONGLONG llSize) { m_llSize += llSize; }
	ULONGLONG GetSize() { return m_llSize; }
	void ResetProgress() { m_llProgress = 0; m_llFileProgress = 0; }
	void AddProgress(ULONGLONG llSize) { m_llProgress += llSize; }
	int GetProgress();
	ULONGLONG* GetProgressAddr() { return &m_llProgress; }

	int CountFiles(const CString& strDstPath);

protected:
	BOOL &m_fCanceled;
	ULONGLONG m_llSize = 0;				// file size sum
	ULONGLONG m_llProgress = 0;			// file size progress
	ULONGLONG m_llFileProgress = 0;		// single file size progress
	static char* s_pCopyBuf;

};


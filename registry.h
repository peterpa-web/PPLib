// registry.h : interface of the CRegistry class
//
#pragma once

#include "EventLog.h"

class CRegistry
{
protected: 
	CStringArray m_saValueName;
	CStringArray m_saValue;
	CDWordArray	 m_dwaType;
	HKEY		 m_hKey;
	CString		 m_strSubKey;

// Attributes
public:
	CRegistry();

// Operations
public:
	void OpenSubKey( HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired );
	HKEY GetKey() { return m_hKey; }
	void CreateSubKey( LPCTSTR lpSubKey, REGSAM samDesired,
		LPDWORD lpdwDisposition = NULL,
		LPTSTR lpClass = NULL,
		DWORD dwOptions = REG_OPTION_NON_VOLATILE,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL );
	static void DeleteKey( HKEY hKey, LPCTSTR lpSubKey );
	void DeleteSubKey( LPCTSTR lpSubKey );
	void CloseKey();

	CString GetString( LPCTSTR lpValueName, DWORD dwType = REG_SZ, BOOL bRequired = TRUE );
	DWORD GetDword( LPCTSTR lpValueName, DWORD dwType = REG_DWORD, BOOL bRequired = TRUE );
	void AddString( LPCTSTR lpValueName, LPCTSTR lpData, DWORD dwType = REG_SZ );
	void AddDword( LPCTSTR lpValueName, DWORD dwData, DWORD dwType = REG_DWORD );
	void AddBinary( LPCTSTR lpValueName, const BYTE *lpData, DWORD cbData );
	void AddBinary( LPCTSTR lpValueName, const CString &str );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegistry)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRegistry();

protected:
	// Helpers
	void ReadSubKey();
};

//----------------------------------
class CEventLog;
class CEventLogException;

class CRegException : public CEventLogException
{
	static DWORD m_dwMsgBase;

protected:
	CString m_strFunction;
	LONG	m_lErrorCode;
	CString m_strSubKey;
	CString m_strValueName;

// Attributes
public:
	DECLARE_DYNAMIC( CRegException )
	CRegException( DWORD dwMsg, LPCTSTR lpFunction, LONG lRc, LPCTSTR lpSubKey, LPCTSTR lpValueName = NULL );
	enum {
		msgRegKey,
		msgRegVal,
		msgRegAdd
	};
	static DWORD GetMsgBase() { return m_dwMsgBase; }
	static void SetMsgBase( DWORD dwMsgBase ) {	m_dwMsgBase = dwMsgBase; }

// Operations
public:
	virtual BOOL GetErrorMessage( LPTSTR lpszError, UINT nMaxError,
						  PUINT pnHelpContext );
	virtual BOOL ReportEvent( 
//		CEventLog* pEventLog,
		WORD wType = EVENTLOG_ERROR_TYPE,
		WORD wCategory = 0 );

// Implementation
public:
	virtual ~CRegException();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

};


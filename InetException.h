// InetException.h: interface of the CInetException class
//
#pragma once

#include "EventLog.h"

class CInetException : public CEventLogException
{
// Attributes
public:
	DECLARE_DYNAMIC( CInetException )
	CInetException( DWORD dwMsg, LPCTSTR pszMsg = NULL );
	CInetException( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg = NULL );
	CInetException( LPCTSTR pszMsg = NULL, DWORD dwLastError = 0 );

// Operations
public:
	virtual BOOL GetErrorMessage( 
		LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext );

	virtual BOOL ReportEvent( 
//		CEventLog* pEventLog = NULL,
		WORD wType = EVENTLOG_ERROR_TYPE,
		WORD wCategory = 0 );

	static CString GetLastErrorText( DWORD dwLastError );
	static CString GetLastErrorText() { return GetLastErrorText( GetLastError() ); }
	int GetExtErrno() { return m_nExtErrno; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinHTTPException)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInetException();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void initMsg2( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg );
	int m_nExtErrno;

};



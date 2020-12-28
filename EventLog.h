// EventLog.h: interface of the CEventLog class
//
#pragma once

class CEventLog
{
// Attributes
protected:
	static CEventLog* s_pEventLog;
//	static DWORD m_dwDefaultMsgBase;	

	CString	m_strUNCServerName;
	CString	m_strSourceName;
	PSID	m_lpUserSid;	// user security identifier (optional) 
	PTOKEN_USER	m_pTokenUser;
	HANDLE	m_hEventLogWrite;

// Constructor
private:
	CEventLog();

public:
	~CEventLog();
	static void CleanStatic() {
		if (s_pEventLog != NULL)
			delete s_pEventLog;
		s_pEventLog = NULL;
	}

// Operations
public:
	static CEventLog& GetInstance();
//	CEventLog( LPCTSTR lpSourceName = NULL,
//			   PSID    lpUserSid = NULL,
//			   LPCTSTR lpUNCServerName = NULL );

	PSID GetUserSid() { return m_lpUserSid; }

//	static CEventLog* GetThis() { return s_pEventLog; }
//	static DWORD GetMsgBase() {	return m_dwDefaultMsgBase; }
//	static void SetMsgBase( DWORD dwMsgBase ) { m_dwDefaultMsgBase = dwMsgBase; }

	static void Init( LPCTSTR lpSourceName = NULL,
			          PSID    lpUserSid = NULL,
			          LPCTSTR lpUNCServerName = NULL );

	void Register( 
		LPCTSTR lpEventMessageFile = NULL,
		DWORD dwTypesSupported = EVENTLOG_ERROR_TYPE |
								 EVENTLOG_WARNING_TYPE |
								 EVENTLOG_INFORMATION_TYPE ,
		DWORD dwCategoryCount = 0,
		LPCTSTR lpCategoryMessageFile = NULL,
		LPCTSTR lpParameterMessageFile = NULL
	);	// throws *CRegException

	BOOL SetUserSid( PSID pUserSid = NULL );

	static BOOL Write(
		WORD wType,			// event type to log 
		WORD wCategory,		// event category 
		DWORD dwEventID,	// event identifier 
		WORD wNumStrings,	// number of strings to merge with message  
		DWORD dwDataSize,	// size of binary data, in bytes
		LPCTSTR *lpStrings,	// array of strings to merge with message 
		LPVOID lpRawData 	// address of binary data 
	);

	static BOOL Write(
		CException* pe,		// ptr to exception
		DWORD dwEventID = 0,	// generic event identifier 
		WORD wType = EVENTLOG_ERROR_TYPE,	// event type to log 
		WORD wCategory = 0		// event category 
	);

	static BOOL Write(
		WORD wType,			// event type to log 
		WORD wCategory,		// event category 
		DWORD dwEventID,	// event identifier 
		CStringArray* paString,	// ptr to array of strings to merge with message
		DWORD dwDataSize = 0,	// size of binary data, in bytes
		LPVOID lpRawData = NULL	// address of binary data 
	);

	static BOOL Write(
		WORD wType,			// event type to log 
		WORD wCategory,		// event category 
		DWORD dwEventID,	// event identifier 
		LPCTSTR pString = NULL,	// ptr to string to merge with message
		DWORD dwDataSize = 0,	// size of binary data, in bytes
		LPVOID lpRawData = NULL	// address of binary data 
	);

};

//********************************************************

class CEventLogException : public CException
{
protected:
	DWORD   m_dwMsg;
	CString m_strMsg;
	static CString m_strContext;

// Attributes
public:
	DECLARE_DYNAMIC( CEventLogException )
	CEventLogException( DWORD dwMsg, LPCTSTR pszMsg = NULL );
	CEventLogException( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg = NULL );
	CEventLogException( LPCTSTR pszMsg = NULL );
	DWORD GetMsgCode() { return m_dwMsg; }

	static void SetContext( CString const & strContext ) { m_strContext = strContext; }
	static CString GetContext() { return m_strContext; }

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

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventLogException)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEventLogException();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void initMsg1( DWORD dwMsg, LPCTSTR pszMsg );
	void initMsg2( DWORD dwMsg, DWORD dwLastError, LPCTSTR pszMsg );
};

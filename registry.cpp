// registry.cpp : implementation of the CRegistry class
//
// Copyright 2006 Peter Pagel
//
// All rights reserved.

#include "stdafx.h"
#include "registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CRegistry

CRegistry::CRegistry( )
{
	m_hKey = NULL;
	m_saValueName.SetSize( 0, 10 );
	m_saValue.SetSize( 0, 10 );
	m_dwaType.SetSize( 0, 10 );
}

CRegistry::~CRegistry()
{
	CloseKey();
}


/////////////////////////////////////////////////////////////////////////////
// CRegistry commands

void CRegistry::OpenSubKey( HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired )
{
	LONG lRc;

	lRc = RegOpenKeyEx( hKey, lpSubKey, 0, samDesired, &m_hKey );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegKey, _T("OpenSubKey"), lRc, lpSubKey );
	m_strSubKey = lpSubKey;
	m_saValueName.RemoveAll();
}

void CRegistry::CreateSubKey( 
	LPCTSTR lpSubKey, 
	REGSAM samDesired, 
	LPDWORD lpdwDisposition,
	LPTSTR lpClass,
	DWORD dwOptions,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes )
{
	LONG lRc;
	HKEY hkResult;

	VERIFY( lpSubKey != NULL );
	DWORD dwDisposition;
	lRc = RegCreateKeyEx( m_hKey, lpSubKey, 0, lpClass, dwOptions,
		samDesired, lpSecurityAttributes, &hkResult, &dwDisposition );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegKey, _T("CreateSubKey"), lRc, m_strSubKey );
	m_hKey = hkResult;
	if ( lpdwDisposition != NULL )
		*lpdwDisposition = dwDisposition;
	m_strSubKey += "\\";
	m_strSubKey += lpSubKey;
	m_saValueName.RemoveAll();
}

void CRegistry::DeleteKey( HKEY hKey, LPCTSTR lpSubKey )
{
	LONG lRc = RegDeleteKey( hKey, lpSubKey );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegKey, _T("DeleteKey"), lRc, lpSubKey );
}

void CRegistry::DeleteSubKey( LPCTSTR lpSubKey )
{
	LONG lRc = RegDeleteKey( m_hKey, lpSubKey );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegKey, _T("DeleteSubKey"), lRc, lpSubKey );
}

void CRegistry::CloseKey()
{
	LONG lRc;

	if ( m_hKey == NULL ) return;
	lRc = RegCloseKey( m_hKey );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegKey, _T("CloseKey"), lRc, m_strSubKey );
	m_hKey = NULL;
	m_strSubKey.Empty();
}

void CRegistry::ReadSubKey()
{
	LONG lRc;
	DWORD dwValues;
	DWORD dwMaxValueNameLen;
	DWORD dwMaxValueLen;
	
	lRc = RegQueryInfoKey( m_hKey, NULL, NULL, NULL, NULL, NULL, NULL,
		&dwValues, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL );

	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegKey, _T("ReadSubKey RegQueryInfoKey"), lRc, m_strSubKey );
	
	m_saValueName.SetSize( dwValues );
	m_saValue.SetSize( dwValues );
	m_dwaType.SetSize( dwValues );
	for ( DWORD n = 0; n < dwValues; n++ )
	{
		DWORD	dwNameSize  = dwMaxValueNameLen + 1;
		DWORD	dwValueSize = dwMaxValueLen;
		CString strValueName;
		CString strValue;
		LPTSTR  lpValueName = strValueName.GetBuffer( dwMaxValueNameLen + 1 );
		LPBYTE  lpValue     = (LPBYTE) strValue.GetBuffer( dwMaxValueLen + 1 );
		DWORD   dwType;

		lRc = RegEnumValue( m_hKey,
			n, lpValueName, &dwNameSize,
			NULL, &dwType, lpValue, &dwValueSize );
		strValueName.ReleaseBuffer( dwNameSize );
		if ( dwType == REG_SZ ||
			 dwType == REG_EXPAND_SZ )
		{
			dwValueSize /= sizeof(TCHAR);	// 20161116
			if ( dwValueSize > 0 ) dwValueSize--;
		}
		strValue.ReleaseBuffer( dwValueSize );
		m_saValueName[ n ] = strValueName;
		m_saValue[ n ]     = strValue;
		m_dwaType[ n ]     = dwType;
		if ( lRc != ERROR_SUCCESS ) 
			throw new CRegException( CRegException::msgRegKey, _T("ReadSubKey RegEnumValue"), lRc, m_strSubKey );
	}
}

CString CRegistry::GetString( 
		LPCTSTR lpValueName, 
		DWORD dwType /* = REG_SZ */, 
		BOOL bRequired /* = TRUE */ )
{
	CString strValue;

	if ( dwType == REG_SZ ||
		 dwType == REG_EXPAND_SZ ||
		 dwType == REG_BINARY )
	{
		int nValues = (int)m_saValueName.GetSize();
		if ( nValues == 0 )
		{
			ReadSubKey();
			nValues = (int)m_saValueName.GetSize();
		}
		for ( int n = 0; n < nValues; n++ ) 
		{
			if ( ( m_dwaType[ n ] == dwType )  &&
				 m_saValueName[ n ].CompareNoCase( lpValueName ) == 0 )
			{
				strValue = m_saValue[ n ];
				return strValue;
			}
		}
	}
	if ( bRequired )
		throw new CRegException( CRegException::msgRegVal, _T("GetString"), 0, m_strSubKey, lpValueName );
	return strValue;
}

DWORD CRegistry::GetDword(
		LPCTSTR lpValueName, 
		DWORD dwType /* = REG_DWORD */, 
		BOOL bRequired /* = TRUE */ )
{
	if ( dwType == REG_DWORD ||
	     dwType == REG_DWORD_LITTLE_ENDIAN )
	{
		int nValues = (int)m_saValueName.GetSize();
		if ( nValues == 0 )
		{
			ReadSubKey();
			nValues = (int)m_saValueName.GetSize();
		}
		for ( int n = 0; n < nValues; n++ ) 
		{
			if ( ( m_dwaType[ n ] == dwType ) &&
				 m_saValueName[ n ].CompareNoCase( lpValueName ) == 0 )
			{
				 LPCTSTR lpValue = m_saValue[ n ];
				 return *(const DWORD *)lpValue;
			}
		}
	}
	if ( bRequired )
		throw new CRegException( CRegException::msgRegVal, _T("GetDword"), 0, m_strSubKey, lpValueName );
	return 0;
}

void CRegistry::AddString( LPCTSTR lpValueName, LPCTSTR lpData, DWORD dwType )
{
	LONG lRc;

	ASSERT( lpValueName != NULL );
	ASSERT( lpData != NULL );
	ASSERT( dwType == REG_SZ ||
		    dwType == REG_EXPAND_SZ );
	DWORD cbData = lstrlen( lpData );
	cbData++;
	cbData *= sizeof( _TCHAR);

	lRc = RegSetValueEx( m_hKey, lpValueName, 0, dwType,
		(CONST BYTE *) lpData, cbData );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegAdd, _T("AddString"), lRc, m_strSubKey, lpValueName );
}

void CRegistry::AddDword( LPCTSTR lpValueName, DWORD dwData, DWORD dwType )
{
	LONG lRc;

	ASSERT( lpValueName != NULL );
	ASSERT( dwType == REG_DWORD ||
		    dwType == REG_DWORD_LITTLE_ENDIAN );
	lRc = RegSetValueEx( m_hKey, lpValueName, 0, dwType,
		(CONST BYTE *) &dwData, sizeof(dwData) );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegAdd, _T("AddDword"), lRc, m_strSubKey, lpValueName );
}

void CRegistry::AddBinary( LPCTSTR lpValueName, const BYTE *lpData, DWORD cbData )
{
	LONG lRc;

	ASSERT( lpValueName != NULL );
	ASSERT( lpData != NULL );
	ASSERT( cbData != 0 );

	lRc = RegSetValueEx( m_hKey, lpValueName, 0, REG_BINARY,
		 lpData, cbData );
	if ( lRc != ERROR_SUCCESS ) 
		throw new CRegException( CRegException::msgRegAdd, _T("AddBinary"), lRc, m_strSubKey, lpValueName );
}

void CRegistry::AddBinary( LPCTSTR lpValueName, const CString &str )
{

	DWORD cbData = str.GetLength();
	cbData *= sizeof( _TCHAR);

	AddBinary( lpValueName, (const BYTE *)(LPCTSTR)str, cbData );
}

/////////////////////////////////////////////////////////////////////////////
// CRegException

IMPLEMENT_DYNAMIC(CRegException, CEventLogException)

DWORD CRegException::m_dwMsgBase = 0;	// static init

CRegException::CRegException( DWORD dwMsg, LPCTSTR lpFunction, LONG lRc, LPCTSTR lpSubKey, LPCTSTR lpValueName ) :
	CEventLogException( dwMsg + m_dwMsgBase )
{
	m_strFunction = lpFunction;
	m_lErrorCode = lRc;
	m_strSubKey   = lpSubKey;
	if ( lpValueName != NULL )
		m_strValueName = lpValueName;
	TRACE3("CRegException::%s SubKey %s Value %s\n",
		lpFunction, lpSubKey, (lpValueName == NULL) ? _T("???") : lpValueName);
}

CRegException::~CRegException()
{
}


/////////////////////////////////////////////////////////////////////////////
// CRegException diagnostics

#ifdef _DEBUG
void CRegException::AssertValid() const
{
	CEventLogException::AssertValid();
}

void CRegException::Dump(CDumpContext& dc) const
{
	CEventLogException::Dump(dc);

	dc << "m_strFunction = " << m_strFunction << "\n";
	dc << "m_lErrorCode = " << m_lErrorCode << "\n";
	dc << "m_strSubKey = " << m_strSubKey << "\n";
	dc << "m_strValueName = " << m_strValueName << "\n";
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRegException commands

BOOL CRegException::GetErrorMessage( LPTSTR lpszError, UINT nMaxError,
									PUINT pnHelpContext )
{
	ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

	if (pnHelpContext != NULL)
		*pnHelpContext = 0;

	if (nMaxError == 0 || lpszError == NULL)
		return FALSE;

	CString strMsg = _T("Error in CRegistry::");
	strMsg += m_strFunction;
	if ( m_lErrorCode != 0 )
	{
		CString strRc;
		strRc.Format( _T(" returned %ld"), m_lErrorCode );
		strMsg += strRc;
	}
	strMsg += _T(" at Subkey ") + m_strSubKey;
	if ( !m_strValueName.IsEmpty() )
		strMsg += _T(" Value ") + m_strValueName;
	lstrcpyn( lpszError, strMsg, nMaxError );
	return TRUE;
}

BOOL CRegException::ReportEvent( 
//			CsniEventLog* pEventLog,
			WORD wType, /* = EVENTLOG_ERROR_TYPE */ 
			WORD wCategory /* = 0 */ )
{
	CString strFkt = _T("CRegistry::") + m_strFunction;
	CString strErr;
	strErr.Format( _T("%d"), m_lErrorCode );
	LPCTSTR alpString[5];
	WORD nStrings;
	switch ( m_dwMsg - m_dwMsgBase )
	{
	case msgRegKey:
		nStrings = 4;
		alpString[0] = strFkt;
		alpString[1] = strErr;
		alpString[2] = m_strSubKey;
		alpString[3] = m_strContext;
		break;
	case msgRegVal:
		nStrings = 4;
		alpString[0] = strFkt;
		alpString[1] = m_strSubKey;
		alpString[2] = m_strValueName;
		alpString[3] = m_strContext;
		break;
	case msgRegAdd:
		nStrings = 5;
		alpString[0] = strFkt;
		alpString[1] = strErr;
		alpString[2] = m_strSubKey;
		alpString[3] = m_strValueName;
		alpString[4] = m_strContext;
		break;
	default:
		return FALSE;
	}
	return ( CEventLog::GetInstance().Write( 
		wType,				// event type to log
		wCategory,			// event category
		m_dwMsg,			// event identifier 
		nStrings,			// number of strings to merge with message
		0,					// size of binary data, in bytes
		alpString,			// array of strings to merge with message 
		NULL ) ) ;			// address of binary data
}



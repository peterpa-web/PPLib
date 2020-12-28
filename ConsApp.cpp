// ConsApp.cpp : Defines the class CConsApp. 
//

#include "stdafx.h"
#include "EventLog.h"
#include "ConsApp.h"

/////////////////////////////////////////////////////////////////////////////
// global

CWinApp theWinApp;

CConsApp* CConsApp::m_pApp = NULL;	// static init

int _tmain( int argc, _TCHAR *argv[] )
{

	CConsApp* pApp = CConsApp::GetThis();
	try
	{
		pApp->Init();
		return pApp->main( argc, argv );
	}
	catch ( CException *pe )
	{
		pApp->WriteError( pe );
		CEventLog::Write( pe );
		pe->Delete();
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CConsApp

CConsApp::CConsApp()
{
	ASSERT( m_pApp == NULL );
	m_pApp = this;
}

void CConsApp::Init()
{
#ifdef _DEBUG
	afxDump.SetDepth( 1 );
#endif

	// initialize MFC
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		throw new CEventLogException( _T("MFC initialization failed") );
	}
}

CString CConsApp::GetModuleFileName() 
{
    TCHAR szPath[MAX_PATH];

    if ( ::GetModuleFileName( NULL, szPath, MAX_PATH ) > 0 )
	{
		return szPath;
	}
	return _T("");
}

CString CConsApp::GetDefaultFileName( LPCTSTR pszExt ) 
{
	CString strName = GetModuleFileName();
	int nPosExt = strName.ReverseFind( '.' );
	if ( nPosExt > 0 )
	{
		return strName.Left( nPosExt+1 ) + pszExt;
	}
	return _T("");
}

CString CConsApp::GetDefaultPath() 
{
	CString strName = GetModuleFileName();
	int nPosDelim = strName.ReverseFind( '\\' );
	if ( nPosDelim > 0 )
	{
		return strName.Left( nPosDelim+1 );
	}
	return _T("");
}

void CConsApp::WriteError( CException *pe )
{
	const UINT nMaxError = 256;
	_TCHAR szError[nMaxError];

	pe->GetErrorMessage( szError, nMaxError );
	TRACE1("%s\n", szError );
	_ftprintf( stderr, _T("%s\n"), szError );
}

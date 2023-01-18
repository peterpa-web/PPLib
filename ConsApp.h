// ConsApp.h : main header file for the CConsApp class
//
#pragma once

// _tmain must be in the main module:
#define	CONS_MAIN int _tmain(int argc, _TCHAR *argv[]) { return ConsMain(argc, argv); }

int ConsMain( int argc, _TCHAR *argv[] );

/////////////////////////////////////////////////////////////////////////////

class CConsApp
{
	static CConsApp* s_pApp;
	friend int ::ConsMain( int argc, _TCHAR *argv[] );

protected:
	static CConsApp* GetThis() { ASSERT( s_pApp ); return s_pApp; }

	virtual void Init();
	virtual int main( int argc, TCHAR *argv[] ) = 0;

public:
	CConsApp();

	static CString GetModuleFileName();
	static CString GetDefaultFileName( LPCTSTR pszExt );
	static CString GetDefaultPath();
	static void WriteError( CException *pe );
};


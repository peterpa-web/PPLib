// ConsApp.h : main header file for the CConsApp class
//
#pragma once

int _tmain( int argc, _TCHAR *argv[] );

/////////////////////////////////////////////////////////////////////////////

class CConsApp
{
	static CConsApp* m_pApp;
	friend int ::_tmain( int argc, _TCHAR *argv[] );

protected:
	static CConsApp* GetThis() { ASSERT( m_pApp ); return m_pApp; }

	virtual void Init();
	virtual int main( int argc, TCHAR *argv[] ) = 0;

public:
	CConsApp();

	static CString GetModuleFileName();
	static CString GetDefaultFileName( LPCTSTR pszExt );
	static CString GetDefaultPath();
	static void WriteError( CException *pe );
};


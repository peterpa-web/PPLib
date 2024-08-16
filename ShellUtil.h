#pragma once
class CShellUtil
{
public:
	static void Explore(const CString& strPath);
	static void Exec(const CString& strPath, bool bHidden = false);
};


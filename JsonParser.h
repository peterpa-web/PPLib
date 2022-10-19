#pragma once
class CJsonParser
{
public:
	virtual bool Parse(CStringA strInput);

protected:
	virtual void StorePair(const CStringA& strContext, const CStringA& strKey, const CStringA& strValue) {}

	CStringA m_strData;
	int m_nStart = 0;

	bool ParseObject(CStringA strContext);
	bool ParseArray(CStringA strContext);
	bool ParsePair(CStringA strContext);
	bool ParseString(CStringA& strResult);
	bool ParseValue(CStringA& strValue);
	void SkipSpaces();
};


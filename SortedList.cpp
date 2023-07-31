#include "stdafx.h"
#include "StringUtil.h"

#include "SortedList.h"

int CSortedStringItem::Compare(const CSortedItem& item) const
{
	const CSortedStringItem& item2 = dynamic_cast<const CSortedStringItem&>(item);
	return CStringUtil::CompareGerNoCase(m_strKey, item2.m_strKey);
}

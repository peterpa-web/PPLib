#include "stdafx.h"
#include "SortedList.h"

int CSortedStringItem::Compare(const CSortedItem& item) const
{
	const CSortedStringItem& item2 = dynamic_cast<const CSortedStringItem&>(item);
	return m_strKey.CompareNoCase(item2.m_strKey);
}

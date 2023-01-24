#pragma once
#include "afxtempl.h"

class CSortedItem
{
	virtual int Compare(const CSortedItem &item) const = 0;
//	virtual bool IsEqual(const CSortedItem &item) const = 0;
//	virtual bool IsLessEqual(const CSortedItem &item) const = 0;
//	virtual bool IsLessThen(const CSortedItem &item) const = 0;
};

class CSortedStringItem : public CSortedItem
{
public:
	CSortedStringItem() {};
	CSortedStringItem(const CString& strKey) : m_strKey(strKey) {};
	const CString& GetKey() const { return m_strKey; }

	virtual int Compare(const CSortedItem &item) const;
protected:
	CString m_strKey;
};

template<class TYPE, class ARG_TYPE = const TYPE&>
class CSortedList : public CList<TYPE, ARG_TYPE>
{
public:
	POSITION InsertSorted(ARG_TYPE item)
	{
		ASSERT(dynamic_cast<const CSortedItem*>(&item) != NULL);
		POSITION pos = CList<TYPE, ARG_TYPE>::GetTailPosition();
		while (pos != NULL)
		{
			ARG_TYPE obj = CList<TYPE, ARG_TYPE>::GetAt(pos);
			int c = obj.Compare(item);
			if (c == 0)
				return NULL;	// already present
			if (c < 0)
				break;
			CList<TYPE, ARG_TYPE>::GetPrev(pos);
		}
		return pos == NULL ? CList<TYPE, ARG_TYPE>::AddHead(item) : CList<TYPE, ARG_TYPE>::InsertAfter(pos, item);
	}

	POSITION FindIndex(INT_PTR nIndex, INT_PTR n, POSITION pos) const
	{
		while (n < nIndex)
		{
			CList<TYPE, ARG_TYPE>::GetNext(pos);
			n++;
		}
		return pos;
	}

	POSITION FindSequential(ARG_TYPE item) const
	{
		POSITION pos = CList<TYPE, ARG_TYPE>::GetHeadPosition();
		while (pos != NULL)
		{
			POSITION p = pos;
			if (CList<TYPE, ARG_TYPE>::GetNext(pos).Compare(item) == 0)
				return p;
		}
		return NULL;
	}

	POSITION FindSorted(ARG_TYPE item) const
	{
		int nEnd = CList<TYPE, ARG_TYPE>::GetCount();
		return FindSorted(item, 0, CList<TYPE, ARG_TYPE>::GetHeadPosition(), nEnd);
	}

	POSITION FindSorted(ARG_TYPE item, INT_PTR nStart, POSITION posStart, INT_PTR nEnd) const
	{
		if (posStart == NULL)
			return NULL;

		if ((nEnd - nStart) < 2)
		{
			ARG_TYPE obj = CList<TYPE, ARG_TYPE>::GetAt(posStart);
			if (obj.Compare(item) == 0)
				return posStart;
			return NULL;
		}
		INT_PTR nMid = (nStart + nEnd) / 2;
		POSITION posMid = FindIndex(nMid, nStart, posStart);
		if (posMid == NULL)
			return NULL;
		ARG_TYPE obj = CList<TYPE, ARG_TYPE>::GetAt(posMid);
		int c = obj.Compare(item);
		if (c == 0)
			return posMid;

		if (c < 0)
			return FindSorted(item, nStart, posStart, nMid);
		return FindSorted(item, nMid, posMid, nEnd);
	}

	POSITION GetNextMatch(POSITION posStart, const CSortedList<TYPE, ARG_TYPE> &list, POSITION &posList)
	{
		if (posStart == NULL && posList == NULL)
		{
			posStart = CList<TYPE, ARG_TYPE>::GetHeadPosition();
			posList = list.GetHeadPosition();
		}
		else
		{
			CList<TYPE, ARG_TYPE>::GetNext(posStart);
			list.GetNext(posList);
		}

		while (posStart != NULL && posList != NULL)
		{
			int c = CList<TYPE, ARG_TYPE>::GetAt(posStart).Compare(list.GetAt(posList));
			if (c == 0)
				return posStart;
			if (c < 0)
				CList<TYPE, ARG_TYPE>::GetNext(posStart);
			else
				list.GetNext(posList);
		}
		return NULL;
	}

};

class CSortedStringList : public CSortedList<CSortedStringItem>
{
};
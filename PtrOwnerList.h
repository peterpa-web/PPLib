#pragma once
#include "afxcoll.h"

class CPtrOwnerList : public CPtrList
{
public:
	~CPtrOwnerList() { RemoveAll(); }
	void RemoveAll() {
		POSITION pos = GetHeadPosition();
		while (pos != NULL)
		{
			POSITION posCurr = pos;
			delete GetNext(pos);
			CPtrList::RemoveAt(posCurr);
		}
	}
	void RemoveAt(POSITION pos) {
		delete GetAt(pos);
		CPtrList::RemoveAt(pos);
	}
	void RemoveHead() {
		POSITION pos = GetHeadPosition();
		if (pos != NULL)
			CPtrList::RemoveAt(pos);
	}
	void RemoveTail() {
		POSITION pos = GetTailPosition();
		if (pos != NULL)
			CPtrList::RemoveAt(pos);
	}
};
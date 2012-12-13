#include "StdAfx.h"
#include ".\refcounter.h"

RefCounter::RefCounter(void):
refCount_(0)
{
}

RefCounter::~RefCounter(void)
{
}

DWORD RefCounter::IncRef()
{
	return (DWORD)InterlockedIncrement((LONG *)&refCount_);
	//ATLTRACE2("+%ld+\n",refCount_);
}

DWORD RefCounter::DecRef()
{
	return (DWORD)InterlockedDecrement((LONG *)&refCount_);
	//ATLTRACE2("-%ld-\n",refCount_);
}

DWORD RefCounter::GetRef()
{
	ATLTRACE2("=%ld=\n",refCount_);
	return refCount_;
}

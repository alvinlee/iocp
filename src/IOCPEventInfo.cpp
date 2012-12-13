#include "StdAfx.h"
#include ".\iocpeventinfo.h"
#include "IOCPEventHandle.h"

IOCPEventInfo::IOCPEventInfo(IOCPEventHandle *eventHandle):
eventHandle(eventHandle),
infoDelete(FALSE)
{
}

IOCPEventInfo::~IOCPEventInfo(void)
{
}

BOOL IOCPEventInfo::CanDelete()
{
	return eventHandle == NULL && handleContextCount.GetRef() == 0 && 
		(BOOL)InterlockedExchange((LONG *)&infoDelete,TRUE) == FALSE;
}
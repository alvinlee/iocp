#include "stdafx.h"
#include "IOCPEventHandle.h"
#include "IOCPEventInfo.h"


IOCPEventHandle::IOCPEventHandle( void )
{
	eventInfo = new IOCPEventInfo(this);
}

IOCPEventHandle::~IOCPEventHandle( void )
{
	eventInfo->eventHandle = NULL;
    if(eventInfo->CanDelete())delete eventInfo;
}
#include "StdAfx.h"
#include "SocketContext.h"
#include "IOCPEventContoller.h"

SocketContext::SocketContext(IOCPEventInfo *eventInfo,EVENTTYPE eventType,
							 int operation ,void *cookie):
IOCPContext(eventInfo,cookie,timeout),
buffer(NULL),
bufferConst(NULL),
size(0),
offset(0),
operation(operation),
eventType(eventType),
wsacode(0)
{
}

SocketContext::SocketContext(IOCPEventInfo *eventInfo,EVENTTYPE eventType,
							 int operation ,void *cookie, 
							 char *buff,DWORD size,DWORD timeout):
IOCPContext(eventInfo,cookie,timeout),
buffer(buff),
bufferConst(NULL),
size(size),
offset(0),
operation(operation),
eventType(eventType)
{
}

SocketContext::SocketContext(IOCPEventInfo *eventInfo,EVENTTYPE eventType,
							 int operation ,void *cookie,
							 const char *buff,DWORD size,DWORD timeout):
IOCPContext(eventInfo,cookie,timeout),
buffer(NULL),
bufferConst(buff),
size(size),
offset(0),
operation(operation),
eventType(eventType)
{
}

SocketContext::~SocketContext(void)
{
	if(buffer)
		delete[] buffer;
}
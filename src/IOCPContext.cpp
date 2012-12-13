#include "StdAfx.h"
#include "iocpcontext.h"
#include "IOCPEventInfo.h"
#include "IOCPEventContoller.h"
#include "IOCPTimerQueue.h"

RefCounter IOCPContext::totalContext;

IOCPContext::IOCPContext(IOCPEventInfo *eventInfo,void *cookie,DWORD timeout):
eventInfo(eventInfo),
cookie(cookie),
timeout(timeout),
expire(0),
done(FALSE),
eventDone(FALSE),
error(FALSE),
code(0)
{
	_ASSERT(eventInfo);
	ZeroMemory(&overlapped,sizeof(overlapped));

	totalContext.IncRef();
	eventInfo->handleContextCount.IncRef();
}

IOCPContext::~IOCPContext(void)
{
	totalContext.DecRef();
	eventInfo->handleContextCount.DecRef();
	if(eventInfo->CanDelete())delete eventInfo;
}

BOOL IOCPContext::Done(BOOL state)
{
	return InterlockedExchange((LONG *)&done,state);
}

BOOL IOCPContext::EventDone(BOOL state)
{
	return InterlockedExchange((LONG *)&eventDone,state);
}

void IOCPContext::PrepareTimeout()
{
	if(timeout > 0){
		expire = GetTickCount() + timeout;

		_ASSERT(eventInfo->eventHandle);
		IOCPTimerQueue::Instance()->AddTimer(this);
	}
}
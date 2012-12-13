#include "StdAfx.h"
#include "iocptimerqueue.h"
#include "IOCPEventHandle.h"
#include "IOCPEventContoller.h"
#include "IOCPContext.h"
#include "IOCPEventInfo.h"

IOCPTimerQueue IOCPTimerQueue::instance_;

IOCPTimerQueue::IOCPTimerQueue(void):expireLatest_(INFINITE)
{
	InitializeCriticalSection(&cs_);
}

IOCPTimerQueue::~IOCPTimerQueue(void)
{
	DeleteCriticalSection(&cs_);
}

void IOCPTimerQueue::AddTimer( IOCPContext *context )
{
    EnterCriticalSection(&cs_);

	TIMEOUTLIST &timeoutList = timeoutHash_[context->timeout];
	contextHash_[context] = timeoutList.insert(timeoutList.end(),context);

	// send a dummy event so iocp loop can adjust timeout value when
	// the iocp loop are hang.
	if(context->expire < expireLatest_)
		IOCPEventContoller::Instance()->PostEvent(
			(IOCPEventHandle *)context->eventInfo->eventHandle,NULL,0);

	LeaveCriticalSection(&cs_);
}

void IOCPTimerQueue::CancelTimer( IOCPContext *context )
{
	EnterCriticalSection(&cs_);
	CONTEXTHASHIT it = contextHash_.find(context);
	if(it != contextHash_.end()){
        timeoutHash_[context->timeout].erase((*it).second);
		contextHash_.erase(it);
	}
	LeaveCriticalSection(&cs_);
}

DWORD IOCPTimerQueue::Schedule()
{
	EnterCriticalSection(&cs_);

	DWORD minExpire = INFINITE;

	for(TIMEOUTHASHIT it = timeoutHash_.begin();it != timeoutHash_.end();++it){
		DWORD listExpire = ScheduleList((*it).second);
		if(listExpire < minExpire)minExpire = listExpire;
	}

	expireLatest_ = minExpire;

	LeaveCriticalSection(&cs_);

	return GetLastestTimeout();
}

DWORD IOCPTimerQueue::GetLastestTimeout()
{
	DWORD now = GetTickCount();

	if(expireLatest_ == INFINITE)return INFINITE;
	else if(expireLatest_ <= now)return 0;

	return expireLatest_ - now;
}

DWORD IOCPTimerQueue::ScheduleList( TIMEOUTLIST &timeoutList )
{
	// fire once ,so event can get change 
	if(timeoutList.empty())return INFINITE;

	for(TIMEOUTIT it = timeoutList.begin();
	it != timeoutList.end();){
		DWORD now = GetTickCount();
		if((*it)->expire > now){
			return (*it)->expire;
		}
		else {
			IOCPContext *context = (*it);
			ProcessTimeout(context);
			contextHash_.erase(context);
			it = timeoutList.erase(it);
		}
	}

	return INFINITE;
}

void IOCPTimerQueue::ProcessTimeout( IOCPContext *context )
{
	if(!context->Done(TRUE)){
		IOCPEventHandle *eventHandle = (IOCPEventHandle *)
			(context->eventInfo->eventHandle);
		if(eventHandle)eventHandle->OnTimeout(context);

		// Io event maybe come latter,can't cancel io event here,
		// so give a condition to judge who come at last.
		if(context->EventDone(TRUE)){
			delete context;
		}
	}
}

IOCPTimerQueue * IOCPTimerQueue::Instance()
{
	return &instance_;
}


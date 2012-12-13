#include "StdAfx.h"
#include "iocpeventcontoller.h"
#include "IOCPEventHandle.h"
#include "IOCPEventInfo.h"
#include "IOCPTimerQueue.h"
#include "IOCPContext.h"

IOCPEventContoller IOCPEventContoller::instance_;

IOCPEventContoller::IOCPEventContoller(void):
shouldExit_(FALSE)
{
	iocpHandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(!iocpHandle_)
	{
		ATLTRACE2("IOCPEventContoller can't create iocp!\n");
	}

	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	suggestThreadCount_ = systemInfo.dwNumberOfProcessors * 2;
}

IOCPEventContoller::~IOCPEventContoller(void)
{
	if(iocpHandle_)CloseHandle(iocpHandle_);
}

BOOL IOCPEventContoller::RegisterHandle( IOCPEventHandle *eventHandle )
{
	if(!iocpHandle_)return FALSE;

	iocpHandle_ = CreateIoCompletionPort(eventHandle->GetIOHandle(),
		iocpHandle_,(ULONG_PTR)eventHandle,0);

	return (iocpHandle_ != NULL);
}

BOOL IOCPEventContoller::DispatchEvent()
{
	DWORD ioSize = 0;
	ULONG_PTR key = NULL;
	LPOVERLAPPED lpOverlapped = NULL;

	DWORD timeout = IOCPTimerQueue::Instance()->Schedule();

	BOOL ret = GetQueuedCompletionStatus(iocpHandle_,&ioSize,
		&key,&lpOverlapped,timeout);

	DWORD error = 0;

	if(!ret){
		error = GetLastError();
		ATLTRACE2("DispatchEvent Error:%ld\n",error);

		if(error == WAIT_TIMEOUT && lpOverlapped == NULL){
			return TRUE;
		}
	}

	if(!key && !lpOverlapped){ // exit message
		ATLTRACE2("DispatchEvent Will Exit!\n");
		shouldExit_ = TRUE;
		return IOCPContext::totalContext.GetRef() > 0;
	}

	if(key && lpOverlapped){
		// ignore vtable
		PULONG_PTR pointer = reinterpret_cast<PULONG_PTR>(lpOverlapped);
		--pointer;
		IOCPContext *context = reinterpret_cast<IOCPContext *>(pointer);
		context->code = error;
		ProcessEvent(context,ioSize,!ret);
	}

	return !shouldExit_ || IOCPContext::totalContext.GetRef() != 0;
}

void IOCPEventContoller::ProcessEvent(IOCPContext *context,DWORD ioSize,BOOL error)
{
	// don't have timeout
	if(context->expire == 0){
		SafeEvent(context,ioSize,error);
	}
	// have time out
	else if(!context->Done(TRUE)){
		// Cancel Timer
		IOCPTimerQueue::Instance()->CancelTimer(context);
		SafeEvent(context,ioSize,error);
	}
	// already done,but timeout maybe processing.
	// Delete context when timeout complete
	// else 
	else if (context->EventDone(TRUE)){
		delete context;
	}
}

void IOCPEventContoller::SafeEvent( IOCPContext *context,DWORD ioSize,BOOL error )
{
	IOCPEventHandle *&eventHandle = (IOCPEventHandle *&)context->eventInfo->eventHandle;
	BOOL ret = TRUE;
	if(eventHandle)ret = eventHandle->OnEvent(context,ioSize,error);
	if(ret)delete context;
}

BOOL IOCPEventContoller::PostEvent( IOCPEventHandle *eventHandle,IOCPContext *context,DWORD iosize )
{
	return PostQueuedCompletionStatus(iocpHandle_, iosize , 
		reinterpret_cast<ULONG_PTR>(eventHandle),
		context ? &context->overlapped : NULL);
}

BOOL IOCPEventContoller::StopProcessing()
{
	return PostQueuedCompletionStatus(iocpHandle_, 0, 0, NULL);
}

int IOCPEventContoller::GetSuggestThreadCount()
{
	return suggestThreadCount_;
}

IOCPEventContoller * IOCPEventContoller::Instance()
{
	return &instance_;
}


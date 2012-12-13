#pragma once

class IOCPContext;
class IOCPEventInfo;

class IOCPEventHandle
{
public:
	IOCPEventHandle(void);
	virtual ~IOCPEventHandle(void);
	
	virtual HANDLE GetIOHandle() = 0;
	virtual BOOL OnEvent(IOCPContext *iocpcontext,DWORD ioSize,BOOL error) = 0;
	virtual void OnTimeout(IOCPContext *iocpcontext) = 0;

protected:
	IOCPEventInfo *eventInfo;
};
#pragma once

class IOCPEventHandle;
class IOCPContext;

class IOCPEventContoller
{
public:
	IOCPEventContoller(void);
	virtual ~IOCPEventContoller(void);
	BOOL RegisterHandle(IOCPEventHandle *eventHandle);
	BOOL DispatchEvent();
	BOOL PostEvent(IOCPEventHandle *eventHandle,IOCPContext *context,DWORD iosize);
	BOOL StopProcessing();
	int GetSuggestThreadCount();
	//void AdjustTimeout(DWORD expire,DWORD now);
	static IOCPEventContoller *Instance();
protected:
	void ProcessEvent(IOCPContext *context,DWORD ioSize,BOOL error);
	void SafeEvent(IOCPContext *context,DWORD ioSize,BOOL error);

	volatile BOOL shouldExit_;
	HANDLE iocpHandle_;
	int suggestThreadCount_;
	static IOCPEventContoller instance_;
};

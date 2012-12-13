#pragma once
#include "iocpeventhandle.h"
#include "StreamRate.h"

class SocketHandle;
class SocketContext;

class SocketEventHandle
{
public:
	virtual void OnEvent(DWORD ioSize,SocketContext *context,BOOL error) = 0;
	virtual void OnTimeout(SocketContext *context) = 0;
	virtual void OnClose() = 0;
};

class SocketHandle :
	public IOCPEventHandle
{
public:

	SocketHandle(SocketEventHandle *eventHandle,SocketEventHandle *continueHandle);
	virtual ~SocketHandle(void);

	void CreateSocket();
	void CloseSocket(BOOL lastChange = FALSE);

	void SetEventHandle(SocketEventHandle *eventHandle);

	virtual HANDLE GetIOHandle();
	virtual BOOL OnEvent(IOCPContext *iocpcontext,DWORD ioSize,BOOL error);
	virtual void OnTimeout(IOCPContext *iocpcontext);

	StreamRate &GetStreamRate();
	SOCKADDR_IN &GetLocalAddr();
	SOCKADDR_IN &GetRemoteAddr();

protected:
	void SafeEvent(SocketContext *context);
	void SafeContinueEvent(SocketContext *context);
	void SafeTimeout(SocketContext *context);
	void SafeClose();

protected:
	volatile SOCKET socket_;
	SocketEventHandle *eventHandle_;
	SocketEventHandle *continueHandle_;
	StreamRate streamRate_;
	SOCKADDR_IN localAddr_;
	SOCKADDR_IN remoteAddr_;
};

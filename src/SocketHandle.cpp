#include "stdafx.h"
#include "sockethandle.h"
#include "IOCPEventContoller.h"
#include "IOCPTimerQueue.h"
#include "SocketContext.h"

SocketHandle::SocketHandle(SocketEventHandle *eventHandle,SocketEventHandle *continueHandle):
socket_(INVALID_SOCKET),
eventHandle_(eventHandle),
continueHandle_(continueHandle)
{
	ZeroMemory(&localAddr_,sizeof(SOCKADDR_IN));
	ZeroMemory(&remoteAddr_,sizeof(SOCKADDR_IN));
	CreateSocket();
}

SocketHandle::~SocketHandle(void)
{
	CloseSocket(TRUE);
}

void SocketHandle::CreateSocket()
{
	if(socket_ != INVALID_SOCKET)CloseSocket();
	socket_ = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
	_ASSERT( socket_ != INVALID_SOCKET );

	//int iMode = 0;
	//ioctlsocket(socket_, FIONBIO, (u_long FAR*) &iMode);

	//LINGER  lingerStruct;
	//lingerStruct.l_onoff = 1;
	//lingerStruct.l_linger = 0;
	//setsockopt(socket_, SOL_SOCKET, SO_LINGER,(char *)&lingerStruct, sizeof(lingerStruct) );

	//int nZero = 102400;
	//int size = sizeof(nZero);
	//setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (char *)&nZero, sizeof(nZero));
	//setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (char *)&nZero, sizeof(nZero));

	//getsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&nZero, &size);
	//getsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (char *)&nZero, &size);

	BOOL reuseAddr = TRUE;
	setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof(reuseAddr));

	//setsockopt(socket_, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&reuseAddr, sizeof(reuseAddr));
	//setsockopt(socket_, SOL_SOCKET, SO_DONTROUTE, (char *)&reuseAddr, sizeof(reuseAddr));

	//BOOL delay = FALSE;
	//setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (char *)&delay, sizeof(delay));

	//int timeout = 10000;
	//int ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	//ret = setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

	IOCPEventContoller::Instance()->RegisterHandle(this);
}

void SocketHandle::CloseSocket(BOOL lastChange)
{
	SOCKET lastSocket = (SOCKET)InterlockedExchange((LONG *)&socket_,INVALID_SOCKET);
	if(lastSocket != INVALID_SOCKET){
		closesocket(lastSocket);

		if(!lastChange){
			// create close context
			SocketContext *closeContext = new SocketContext(eventInfo,
				SocketContext::CLOSE,SocketContext::CLOSE,NULL);

			// send close context
			IOCPEventContoller::Instance()->PostEvent(this,closeContext,0);
		}
	}
}

void SocketHandle::SetEventHandle( SocketEventHandle *eventHandle )
{
    eventHandle_ = eventHandle;
}

HANDLE SocketHandle::GetIOHandle()
{
	return reinterpret_cast<HANDLE>(socket_);
}

BOOL SocketHandle::OnEvent( IOCPContext *iocpcontext,DWORD ioSize,BOOL error/*=FALSE*/ )
{
	SocketContext *context = static_cast<SocketContext *>(iocpcontext);
	// ATLTRACE2("0x%x %ld %ld %ld\n",context,context->size,context->offset,ioSize);

	switch(context->eventType)
	{
	case SocketContext::RECV:
		streamRate_.AddInBytes(ioSize);
		break;
	case SocketContext::SEND:
		streamRate_.AddOutBytes(ioSize);
		break;
	case SocketContext::ACCEPT:
		{
			SocketHandle *session = reinterpret_cast<SocketHandle *>(context->cookie);
			SOCKADDR_IN &localAddr = session->GetLocalAddr();
			SOCKADDR_IN &remoteAddr = session->GetRemoteAddr();

			SOCKADDR_IN *plocal = (SOCKADDR_IN *)(context->buffer + 10);
			SOCKADDR_IN *premote = (SOCKADDR_IN *)(context->buffer + 38);
			localAddr = *plocal;
			remoteAddr = *premote;
		}
		break;
	case SocketContext::CLOSE:
		SafeClose();
		return TRUE;
	default:
		break;
	}

	// continue 
	context->offset += ioSize;
	context->error = context->error || error;

	if(!context->error && ioSize > 0 && context->offset < context->size){
		context->Done(FALSE);
		context->EventDone(FALSE);
		SafeContinueEvent(context);
		return FALSE;
	}

	// event ready
	SafeEvent(context);
	return TRUE;
}

void SocketHandle::OnTimeout(IOCPContext *iocpcontext)
{
	SocketContext *context = static_cast<SocketContext *>(iocpcontext);
	SafeTimeout(context);
}

void SocketHandle::SafeEvent( SocketContext *context )
{
	if(socket_ != INVALID_SOCKET && eventHandle_)
		eventHandle_->OnEvent(context->offset,context,context->error);
}

void SocketHandle::SafeContinueEvent( SocketContext *context )
{
	if(socket_ != INVALID_SOCKET && continueHandle_)
			continueHandle_->OnEvent(context->offset,context,context->error);
}

void SocketHandle::SafeTimeout( SocketContext *context )
{
	if(socket_ != INVALID_SOCKET && eventHandle_)
		eventHandle_->OnTimeout(context);
}

void SocketHandle::SafeClose()
{
	if(socket_ == INVALID_SOCKET && eventHandle_)
		eventHandle_->OnClose();
}

StreamRate & SocketHandle::GetStreamRate()
{
	return streamRate_;
}

SOCKADDR_IN & SocketHandle::GetLocalAddr()
{
	return localAddr_;
}

SOCKADDR_IN & SocketHandle::GetRemoteAddr()
{
    return remoteAddr_;
}
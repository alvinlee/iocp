#include "stdafx.h"
#include "socketsession.h"
#include "SocketContext.h"
#include "IOCPEventContoller.h"
#include "IOCPTimerQueue.h"

SocketSession::SocketSession(SocketEventHandle *eventHandle):SocketHandle(eventHandle,this)
{
}

SocketSession::~SocketSession(void)
{
}

BOOL SocketSession::SendBuff( char *buff,DWORD size,int operation,void *cookie /*= NULL*/,DWORD timeout /*= 0*/ )
{
	if(socket_ == INVALID_SOCKET)return SOCKET_ERROR;
	SocketContext *context = new SocketContext(eventInfo,SocketContext::SEND,
		operation,cookie,buff,size,timeout);

	WSABUF wsabuff;
	wsabuff.buf = buff;
	wsabuff.len = size;

	return SendContext(&wsabuff,1,context);
}

BOOL SocketSession::SendBuff( const char *buff,DWORD size,int operation,void *cookie,DWORD timeout)
{
	if(socket_ == INVALID_SOCKET)return SOCKET_ERROR;
	SocketContext *context = new SocketContext(eventInfo,SocketContext::SEND,
		operation,cookie,buff,size,timeout);

	WSABUF wsabuff;
	wsabuff.buf = (char *)buff;
	wsabuff.len = size;

	return SendContext(&wsabuff,1,context);
}

BOOL SocketSession::RecvBuff( char *buff,DWORD size,int operation,void *cookie,DWORD timeout)
{
	if(socket_ == INVALID_SOCKET)return SOCKET_ERROR;
	SocketContext *context = new SocketContext(eventInfo,SocketContext::RECV,
		operation,cookie,buff,size,timeout);

	WSABUF wsabuff;
	wsabuff.buf = buff;
	wsabuff.len = size;

	return RecvContext(&wsabuff,1,context);
}

BOOL SocketSession::RecvBuff( const char *buff,DWORD size,int operation,void *cookie,DWORD timeout)
{
	if(socket_ == INVALID_SOCKET)return SOCKET_ERROR;
	SocketContext *context = new SocketContext(eventInfo,SocketContext::RECV,
		operation,cookie,buff,size,timeout);

	WSABUF wsabuff;
	wsabuff.buf = (char *)buff;
	wsabuff.len = size;

	return RecvContext(&wsabuff,1,context);
}


//int SocketSession::SendWSABUF( LPWSABUF lpBuffers,DWORD dwBufferCount,int operation,void *cookie /*= NULL*/ )
//{
//	SocketContext *context = CreateContext(operation,cookie);
//	return SendContext(lpBuffers,dwBufferCount,context);
//}
//
//int SocketSession::RecvWSABUF( LPWSABUF lpBuffers,DWORD dwBufferCount,int operation,void *cookie /*= NULL*/ )
//{
//	SocketContext *context = CreateContext(operation,cookie);
//	return RecvContext(lpBuffers,dwBufferCount,context);
//}

BOOL SocketSession::SendContext( LPWSABUF lpBuffers,DWORD dwBufferCount,SocketContext *context )
{
	DWORD dwNumBytes = 0;
	DWORD dwFlags = 0;

	context->eventType = SocketContext::SEND;

	context->PrepareTimeout();
	int ret = WSASend(socket_,lpBuffers,dwBufferCount,
		&dwNumBytes,dwFlags,&context->overlapped,NULL);

	context->code = WSAGetLastError();

	if(ret == SOCKET_ERROR &&  context->code != WSA_IO_PENDING){
		context->error = TRUE;
		IOCPEventContoller::Instance()->PostEvent(this,context,0);
		return FALSE;
	}

	return TRUE;
}

BOOL SocketSession::RecvContext( LPWSABUF lpBuffers,DWORD dwBufferCount,SocketContext *context )
{
	DWORD dwNumBytes = 0;
	DWORD dwFlags = 0;

	context->eventType = SocketContext::RECV;

	context->PrepareTimeout();
	int ret = WSARecv(socket_,lpBuffers,dwBufferCount,
		&dwNumBytes,&dwFlags,&context->overlapped,NULL);

	context->code = GetLastError();
	context->wsacode = WSAGetLastError();

	if(ret == SOCKET_ERROR && context->wsacode != WSA_IO_PENDING){
		context->error = TRUE;
		IOCPEventContoller::Instance()->PostEvent(this,context,0);
		return FALSE;
	}

	return TRUE;
}

void SocketSession::OnEvent( DWORD ioSize,SocketContext *context,BOOL error )
{
	WSABUF wsabuff;

	if(context->buffer)wsabuff.buf = context->buffer + context->offset;
	else if(context->bufferConst)wsabuff.buf = (char *)(context->bufferConst + context->offset);
	
	wsabuff.len = context->size - context->offset;

	switch(context->eventType)
	{
	case SocketContext::RECV:
		RecvContext(&wsabuff,1,context);
		break;
	case SocketContext::SEND:
		SendContext(&wsabuff,1,context);
	    break;
	default:
	    break;
	}
}
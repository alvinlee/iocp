#include "stdafx.h"
#include "socketaccepter.h"
#include "SocketSession.h"
#include "SocketContext.h"

SocketAccepter::SocketAccepter(SocketEventHandle *eventHandle):SocketHandle(eventHandle,NULL)
{
	DWORD bytes = 0;
	GUID acceptex_guid = WSAID_ACCEPTEX;

	if (WSAIoctl(socket_,SIO_GET_EXTENSION_FUNCTION_POINTER,
		&acceptex_guid,sizeof(acceptex_guid),
		&fnAcceptEx_,sizeof(fnAcceptEx_),
		&bytes,NULL,NULL) == SOCKET_ERROR)
	{
		ATLTRACE2("failed to load AcceptEx: %d\n", WSAGetLastError());
	}
}

SocketAccepter::~SocketAccepter(void)
{
}

BOOL SocketAccepter::Listen( u_short port )
{
	int nRet = 0;

	SOCKADDR_IN &addr = GetLocalAddr();
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	nRet = bind(socket_, (LPSOCKADDR)&addr, sizeof(addr));
	if( nRet != 0 ) return FALSE;

	return listen(socket_, 5) == 0;
}

BOOL SocketAccepter::Accept( SocketSession *session )
{
	_ASSERT(session);
	SocketContext *context = new SocketContext(eventInfo,
		SocketContext::ACCEPT,SocketContext::ACCEPT,session);

	int addrsize = sizeof(SOCKADDR_IN) + 16;
	context->buffer = new char[addrsize*2];

	BOOL ret = fnAcceptEx_(socket_,reinterpret_cast<SOCKET>(session->GetIOHandle()),
		context->buffer,0,addrsize,addrsize,NULL, &(context->overlapped));

	if( ret == FALSE &&  WSAGetLastError() != ERROR_IO_PENDING)return FALSE;

	return TRUE;
}

SocketSession * SocketAccepter::Accept(SocketEventHandle *eventHandle)
{
	SocketSession *session = new SocketSession(eventHandle);
	if(Accept(session))return session;

	delete session;
	return NULL;
}
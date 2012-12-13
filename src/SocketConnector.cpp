#include "stdafx.h"
#include "socketconnector.h"
#include "SocketSession.h"

SocketConnector::SocketConnector(void)
{
}

SocketConnector::~SocketConnector(void)
{
}

BOOL SocketConnector::Connect( SocketSession *session,const SOCKADDR_IN &addr )
{
	_ASSERT(session);
	int ret = connect((SOCKET)session->GetIOHandle(),(SOCKADDR *)&addr,sizeof(addr));
	//int errcode = WSAGetLastError();

	if(ret != SOCKET_ERROR){
		session->GetRemoteAddr() = addr;
        SOCKADDR_IN &remoteAddr = session->GetLocalAddr();
		int addsize = sizeof(remoteAddr);
		getsockname((SOCKET)session->GetIOHandle(),
			(SOCKADDR *)&remoteAddr,&addsize);
	}

	return ret != SOCKET_ERROR;
}

SocketSession * SocketConnector::Connect( SocketEventHandle *eventHandle,const SOCKADDR_IN &addr )
{
	SocketSession *session = new SocketSession(eventHandle);
	if(Connect(session,addr))return session;

	delete session;

	return NULL;
}

//BOOL SocketConnector::ConnectEx( SocketSession *session,const SOCKADDR_IN &addr )
//{
//	_ASSERT(session);
//	int ret = ::ConnectEx((SOCKET)session->GetIOHandle(),(SOCKADDR *)&addr,sizeof(addr),NULL,0,NULL,NULL);
//	int errcode = WSAGetLastError();
//	return ret != SOCKET_ERROR || WSAGetLastError() == WSA_IO_PENDING ;
//}
//
//SocketSession * SocketConnector::ConnectEx( SocketEventHandle *eventHandle,const SOCKADDR_IN &addr )
//{
//	SocketSession *session = new SocketSession(eventHandle);
//	if(ConnectEx(session,addr))return session;
//
//	delete session;
//
//	return NULL;
//}
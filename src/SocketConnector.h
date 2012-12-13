#pragma once

class SocketSession;
class SocketEventHandle;

class SocketConnector
{
public:
	static BOOL Connect(SocketSession *session,const SOCKADDR_IN &addr);
	static SocketSession *Connect(SocketEventHandle *eventHandle,const SOCKADDR_IN &addr);

	//static BOOL ConnectEx(SocketSession *session,const SOCKADDR_IN &addr);
	//static SocketSession *ConnectEx(SocketEventHandle *eventHandle,const SOCKADDR_IN &addr);
private:
	SocketConnector(void);
	~SocketConnector(void);
};

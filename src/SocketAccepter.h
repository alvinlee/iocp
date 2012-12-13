#pragma once

#include <MSWSock.h>
#include "sockethandle.h"

class SocketSession;

class SocketAccepter :
	public SocketHandle
{
public:
	BOOL Listen(u_short port);

	BOOL Accept(SocketSession *session);
	SocketSession *Accept(SocketEventHandle *eventHandle);

	SocketAccepter(SocketEventHandle *eventHandle);
	virtual ~SocketAccepter(void);

protected:
	LPFN_ACCEPTEX fnAcceptEx_;
};

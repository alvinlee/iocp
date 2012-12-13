#pragma once

#include "IOCPContext.h"

class SocketContext : public IOCPContext
{
friend class SocketContextPool;
public:
	typedef enum{
		UNKNOW = 0,
		ACCEPT,
		SEND,
		RECV,
		CLOSE
		// CONNECT,
	}EVENTTYPE;

	SocketContext(IOCPEventInfo *eventInfo,EVENTTYPE eventType,
		int operation ,void *cookie);
	SocketContext(IOCPEventInfo *eventInfo,EVENTTYPE eventType,
		int operation,void *cookie,char *buff,DWORD size,DWORD timeout);
	SocketContext(IOCPEventInfo *eventInfo,EVENTTYPE eventType,
		int operation,void *cookie,const char *buff,DWORD size,DWORD timeout);

	virtual ~SocketContext(void);

public:
	EVENTTYPE eventType;
	int	operation;

	char *buffer;
	const char *bufferConst;
	DWORD size;
	DWORD offset;
	DWORD wsacode;
};


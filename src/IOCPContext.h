#pragma once

#include "RefCounter.h"

class IOCPEventInfo;

class IOCPContext
{
public:
	IOCPContext(IOCPEventInfo *eventInfo,void *cookie,DWORD timeout);
	virtual ~IOCPContext(void);

	BOOL Done(BOOL state);
	BOOL EventDone(BOOL state);
	void PrepareTimeout();

public:
	WSAOVERLAPPED overlapped;
	IOCPEventInfo *eventInfo;
	void *cookie;

	DWORD timeout;
	DWORD expire;

	// To sync event and timeout,
	// so it can release safely 
	// between io event or timeout at last,
	// done means who trigger the event
	// event done means who end the event
	volatile BOOL done;
	volatile BOOL eventDone;

	BOOL error;
	DWORD code;

    static RefCounter totalContext;
};

#pragma once

#include "RefCounter.h"

class IOCPEventHandle;

class IOCPEventInfo
{
public:
	IOCPEventInfo(IOCPEventHandle *eventHandle);
	~IOCPEventInfo(void);

	BOOL CanDelete();

public:
	volatile IOCPEventHandle *eventHandle;
	volatile BOOL infoDelete;
	RefCounter handleContextCount;
};

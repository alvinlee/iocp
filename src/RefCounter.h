#pragma once

class RefCounter
{
public:
	RefCounter(void);
	~RefCounter(void);

	DWORD IncRef();
	DWORD DecRef();
	DWORD GetRef();
private:
	volatile DWORD refCount_;
};

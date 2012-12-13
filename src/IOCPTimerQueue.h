#pragma once

class IOCPContext;

class IOCPTimerQueue
{
public:
	typedef std::list<IOCPContext *> TIMEOUTLIST;
	typedef TIMEOUTLIST::iterator TIMEOUTIT;
	typedef stdext::hash_map<DWORD,TIMEOUTLIST> TIMEOUTHASH;
	typedef TIMEOUTHASH::iterator TIMEOUTHASHIT;
	typedef stdext::hash_map<IOCPContext *,TIMEOUTIT> CONTEXTHASH;
	typedef CONTEXTHASH::iterator CONTEXTHASHIT;

	IOCPTimerQueue(void);
	~IOCPTimerQueue(void);

	void AddTimer(IOCPContext *context);
	void CancelTimer(IOCPContext *context);

	DWORD Schedule();

	static IOCPTimerQueue *Instance();
private:
	DWORD GetLastestTimeout();
	DWORD ScheduleList(TIMEOUTLIST &timeoutList);
	void ProcessTimeout(IOCPContext *context);

    volatile BOOL scheduling_;
    volatile DWORD expireLatest_;

	CRITICAL_SECTION cs_;
	TIMEOUTHASH timeoutHash_;
	CONTEXTHASH contextHash_;

	static IOCPTimerQueue instance_;
};

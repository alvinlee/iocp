#pragma once

// ! not thread safe

class SocketContext;

class SocketContextPool
{
public:
	typedef std::list<SocketContext *> CONTEXTLIST;
	SocketContextPool(void);
	~SocketContextPool(void);

	void SetMaxsize(DWORD maxsize);

	void Put(SocketContext *&context);
	SocketContext *Get();

private:
	DWORD maxsize_;
	CONTEXTLIST contextList_;
};

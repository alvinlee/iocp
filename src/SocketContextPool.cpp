#include "StdAfx.h"
#include "socketcontextpool.h"
#include "SocketContext.h"

SocketContextPool::SocketContextPool(void):
maxsize_(100)
{
}

SocketContextPool::~SocketContextPool(void)
{
	for(CONTEXTLIST::iterator it = contextList_.begin();
		it != contextList_.end();++it){
        delete *it;
	}
}

void SocketContextPool::SetMaxsize( DWORD maxsize )
{
	maxsize_ = maxsize;
}

void SocketContextPool::Put( SocketContext *&context )
{
	if(contextList_.size() < maxsize_ ){
//		context->Reset();
		contextList_.push_back(context);
	}
	else {
		delete context;
	}

	context = NULL;
}

SocketContext * SocketContextPool::Get()
{
	if (contextList_.empty()){
		// return new SocketContext();
	}

	SocketContext * context = contextList_.front();
	contextList_.pop_front();
	return context;
}
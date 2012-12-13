// echoserver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\SocketAccepter.h"
#include "..\SocketConnector.h"
#include "..\SocketSession.h"
#include "..\IOCPEventContoller.h"
#include "..\SocketContext.h"

#include <vector>

// --------------------------------------------------------------------------------------
const int OPERATION_TEST_REQ = 1;
const int OPERATION_TEST_RESP = 2;

volatile DWORD event_count = 0;
DWORD tick_begin = GetTickCount();
DWORD tick_end = 0;
DWORD buff_size = 10;
DWORD timeout = 0;
DWORD once_count = 1;
DWORD limit_connection = 0;
DWORD limit_echo = 0;
BOOL show_info = TRUE;

// --------------------------------------------------------------------------------------
class SessionEvent : 
	public SocketEventHandle
{
public:
	SocketSession *session;
	std::string buff;
	volatile DWORD count;
	BOOL exit;
	DWORD retry;

	SessionEvent(SocketSession *psession):session(psession),
	buff(buff_size,'6'),count(0),exit(FALSE),retry(100){
		_ASSERT(session);
		session->SetEventHandle(this);
		session->GetStreamRate().Enable(TRUE);
	}

	~SessionEvent(){
		_ASSERT(session);
		delete session;
	}

	void BeginSend(){
		_ASSERT(session);
		for(DWORD i = 0;i < once_count;i++){
			session->SendBuff(buff.c_str(),buff.size(),OPERATION_TEST_REQ,NULL,timeout/**(i+1)*/);
		}
	}

	void BeginRead(){
		_ASSERT(session);
		for(DWORD i = 0;i < once_count;i++){
			session->RecvBuff(buff.c_str(),buff.size(),OPERATION_TEST_RESP,NULL,timeout/**(i+1)*/);
		}
	}

	virtual void OnEvent(DWORD ioSize,SocketContext *context,BOOL error){
		_ASSERT(session);
		if(exit || (limit_echo && count >= limit_echo))return;

		if((DWORD)InterlockedIncrement((LONG *)&count) % 10000 == 0 && show_info)
			std::cout<< this << " " << context 
			<< " session event count:"<<count <<" iosize:"<<ioSize
			<< " timeout:"<<context->timeout<<std::endl
			<< " avg out rate:"<<session->GetStreamRate().GetAvgOutRate()
			<< " avg in rate:"<<session->GetStreamRate().GetAvgInRate()
			<< " cur out rate:"<<session->GetStreamRate().GetCurOutRate()
			<< " cur in rate:"<<session->GetStreamRate().GetCurInRate()<<std::endl;

		if(error || ioSize == 0){
			if(show_info)std::cout<< this<< " " << context
				<< " OnEvent Error!" << ioSize
				<< " : " << context->code 
				<< " : " << context->wsacode << std::endl;
			if(retry++ < 10){
				std::cout<< this<< " " << context
					<< " Retry:" << retry << ".......";
			}
			else {
				session->CloseSocket();
				return;
			}
		}

		BOOL ret = FALSE;

		switch(context->operation)
		{
		case OPERATION_TEST_REQ:
			ret = session->RecvBuff(context->bufferConst,context->size,OPERATION_TEST_RESP,NULL,timeout);
			if(show_info && !ret)std::cout<< this<< " RecvBuff Error!"<<std::endl;
			break;
		case OPERATION_TEST_RESP:
			ret = session->SendBuff(context->bufferConst,context->size,OPERATION_TEST_REQ,NULL,timeout);
			if(show_info && !ret)std::cout<< this<< " SendBuff Error!"<<std::endl;
			break;
		default:
			_ASSERT(FALSE);
		}

	}

	virtual void OnTimeout(SocketContext *context){
		if(show_info)std::cout << this << " " << context << " session timeout!" << context->timeout << std::endl;

		// close it when first timeout
		//if(session->IsLastContext(context))
		session->CloseSocket();
	};

	virtual void OnClose(){
		if(show_info)std::cout << this << " session closed!" << std::endl;
		//delete this;
	};
};

// --------------------------------------------------------------------------------------
class SessionCollection{
public:
	typedef std::list<SessionEvent *> SESSIONLIST;
	SESSIONLIST session_list;

	void recvall(){
		for (SESSIONLIST::iterator it = session_list.begin();
			it != session_list.end();++it){
				(*it)->BeginRead();
			}
	}

	void sendall(){
		for (SESSIONLIST::iterator it = session_list.begin();
			it != session_list.end();++it){
				(*it)->BeginSend();
			}
	}

	void closeall(){
		for (SESSIONLIST::iterator it = session_list.begin();
			it != session_list.end();++it){
				(*it)->session->CloseSocket();
			}
	}

	~SessionCollection(){
		for (SESSIONLIST::iterator it = session_list.begin();
		it != session_list.end();++it){
			delete (*it);
		}
	}
};

class AcceptEvent : 
	public SocketEventHandle
{
public:
	SessionCollection sessions;
	SocketAccepter acceptor;
	volatile DWORD count;

	AcceptEvent():acceptor(this),count(0){}

	void acceptnew(){
		if(!acceptor.Accept((SocketEventHandle *)NULL)){
			if(show_info)std::cout << this << " accept failed!"<<std::endl;
		}
	}

	void init(int port){
		if(acceptor.Listen(port)){
			acceptnew();
		}else{
			if(show_info)std::cout << this << " listen failed!"<<std::endl;
		}
	}

	void fini(){
		sessions.closeall();
		acceptor.CloseSocket();
	}

	virtual void OnEvent(DWORD ioSize,SocketContext *context,BOOL error){
		_ASSERT(context->operation == SocketContext::ACCEPT);
		_ASSERT(context->eventType == SocketContext::ACCEPT);

		if(++count % 1000 == 0 && show_info)std::cout<< this << " accept event count:"<<count<<std::endl;

		SocketSession *session = reinterpret_cast<SocketSession *>(context->cookie);
		HANDLE sessionHandle = session->GetIOHandle();
		setsockopt ((SOCKET)acceptor.GetIOHandle(),
			SOL_SOCKET,	SO_UPDATE_ACCEPT_CONTEXT,
			(char *) sessionHandle, sizeof (sessionHandle));


		if(!error){
			SessionEvent *event = new SessionEvent(session);
			sessions.session_list.push_back(event);
			event->BeginRead();

			acceptnew();
		}
		else delete session;

	}

	virtual void OnTimeout(SocketContext *context){};

	virtual void OnClose(){
		if(show_info)std::cout << this << " " << &acceptor
			<< " acceptor closed!" << std::endl;
	};
};

// --------------------------------------------------------------------------------------

DWORD WINAPI ConnectWorkerThread (LPVOID WorkThreadContext);

class ConnectEvent
{
public:
	SessionCollection sessions;
	std::string addr;
	int port;
	HANDLE thread;
	volatile DWORD count;
	BOOL exit;
	BOOL beginsendinthread;

	ConnectEvent():addr("127.0.0.1"),port(8888),thread(INVALID_HANDLE_VALUE),
	count(0),exit(FALSE),beginsendinthread(FALSE){}

	BOOL connect(BOOL beginsend){
		sockaddr_in clientService; 
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr( addr.c_str() );
		clientService.sin_port = htons( port );

		SocketSession *session = new SocketSession(NULL);
		SessionEvent *event = new SessionEvent(session);

		if(!SocketConnector::Connect(session,clientService)){
			if(show_info)std::cout<<this<<" connect failed!"<<std::endl;
			delete event;
			return FALSE;
		}

		if(++count % 100 == 0 && show_info)
			std::cout<<this<<" connect count:"<<count<<std::endl;
		
		sessions.session_list.push_back(event);
		if(beginsend)event->BeginSend();
		return TRUE;
	}

	void test_connect(BOOL beginsend){
		beginsendinthread = beginsend;
		thread = CreateThread(NULL, 0, ConnectWorkerThread , (LPVOID)this, 0, NULL);
	}

	void test_echo(){
		connect(TRUE);
	}

	void fini(){
		exit = TRUE;
		if(thread != INVALID_HANDLE_VALUE)
            WaitForSingleObject(thread,INFINITE);
		
		sessions.closeall();
	}
};

DWORD WINAPI ConnectWorkerThread (LPVOID WorkThreadContext)
{
	ConnectEvent *event = (ConnectEvent *)WorkThreadContext;

	while(!event->exit && !(limit_connection && event->count > limit_connection)){
		if(!event->connect(event->beginsendinthread)){
			//break;
		}
	}

	return 0;
}

// --------------------------------------------------------------------------------------
DWORD WINAPI EventThread (LPVOID WorkThreadContext)
{
	while(IOCPEventContoller::Instance()->DispatchEvent()){
		event_count++;
	}

	// post stop event let next thread exit.
	IOCPEventContoller::Instance()->StopProcessing();

	return TRUE;
}

class EventThreadPool
{
public:
	int thread_count;
	std::vector<HANDLE> threads;
	EventThreadPool():thread_count(1){}

	void init()
	{
		thread_count = IOCPEventContoller::Instance()->GetSuggestThreadCount();
		for(int i = 0;i < thread_count;i++)
		{
			HANDLE handle = CreateThread(NULL, 0, EventThread , (LPVOID)this, 0, NULL);
			threads.push_back(handle);
		}
	}

	void fini()
	{
		IOCPEventContoller::Instance()->StopProcessing();

		WaitForMultipleObjects(threads.size(),&*threads.begin(),TRUE,INFINITE);
	}
};

// --------------------------------------------------------------------------------------
void showmenu()
{
	std::cout<<":s port => start a echo server."<<std::endl;
	std::cout<<":c ip port => repeat connecting to server ."<<std::endl;
	std::cout<<":e ip port => start a connection and repeat echo."<<std::endl;
	std::cout<<":a ip port => repeat connecting and repeat echo."<<std::endl;
	std::cout<<":g => begin send on last connection collection."<<std::endl;
	std::cout<<":h => begin recive on last connection collection."<<std::endl;
	std::cout<<":b size => set buffer size."<<std::endl;
	std::cout<<":t timeout => set time out."<<std::endl;
	std::cout<<":i count => set the count of 'recv/send' fired at once at beginning."<<std::endl;
	std::cout<<":n limit => the limition of client's connection."<<std::endl;
	std::cout<<":l limit => the limition of client's echos."<<std::endl;
	std::cout<<":z time => sleep for a while."<<std::endl;
	std::cout<<":v => view connections/events record."<<std::endl;
	std::cout<<":r => reset connections/events record."<<std::endl;
	std::cout<<":d => switch output info."<<std::endl;
	std::cout<<":x => exit."<<std::endl;
	std::cout<<":? => help."<<std::endl;
}

std::string getstr()
{
	std::string str(1024,0);
	while(!(std::cin >> &*str.begin())){
		std::cin.clear();
	}

	str.resize(strlen(&*str.begin()));

	return str;
}

int getnum()
{
	int num = 0;
	char bad;
	while(!(std::cin >> num)){
		std::cin.clear();
		std::cin >> bad;
	}
	return num;
}

void reset(){
	tick_begin = GetTickCount();
	event_count = 0;
	StreamRate::Total.Reset();
}

void view(){
    tick_end = GetTickCount();
	DWORD during = tick_end - tick_begin;

	std::cout 
		<< "Total event count: " << event_count
		<< " Time during: " << during
		<< " Tps: ";
	if(during / 1000 > 0)std::cout << event_count/(during / 1000)<< std::endl;
	else std::cout << "waiting.." << std::endl;

	std::cout 
		<< " avg out rate:"<<StreamRate::Total.GetAvgOutRate()
		<< " avg in rate:"<<StreamRate::Total.GetAvgInRate()
		<< " cur out rate:"<<StreamRate::Total.GetCurOutRate()
		<< " cur in rate:"<<StreamRate::Total.GetCurInRate()<<std::endl;
}

// --------------------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	int lastRetCode = WSAStartup( MAKEWORD(2,2), &wsaData );

	EventThreadPool pool;
	pool.init();

	typedef std::list<AcceptEvent *> AEVENTLIST;
	typedef std::list<ConnectEvent *> CEVENTLIST;
	AEVENTLIST accept_event_list;
	CEVENTLIST connect_event_list;

	showmenu();

	StreamRate::Total.Enable(TRUE);
	SessionCollection *last = NULL;

	char wait;
	do{
		std::cout<<":";
		std::cin.clear();
		while(!(std::cin >> wait));

		switch(wait)
		{
		case 's':
			{
				AcceptEvent *listen = new AcceptEvent();

				listen->init(getnum());
				accept_event_list.push_back(listen);
				
				last = &listen->sessions;
			}
			break;
		case 'c':
			{
				ConnectEvent *connect = new ConnectEvent ();
				connect->addr = getstr();
				connect->port = getnum();
				connect->test_connect(FALSE);
				connect_event_list.push_back(connect);

				last = &connect->sessions;
			}

			break;
		case 'e':
			{
				ConnectEvent *connect = new ConnectEvent ();
				connect->addr = getstr();
				connect->port = getnum();
				connect->test_echo();
				connect_event_list.push_back(connect);

				last = &connect->sessions;
			}
		    break;
		case 'a':
			{
				ConnectEvent *connect = new ConnectEvent ();
				connect->addr = getstr();
				connect->port = getnum();
				connect->test_connect(TRUE);
				connect_event_list.push_back(connect);

				last = &connect->sessions;
			}
			break;

		case 'g':
			if(last)last->sendall();
			break;
		case 'h':
			if(last)last->recvall();
			break;

		case 'b':
			buff_size = getnum();
			break;
		case 't':
			timeout = getnum();
		    break;
		case 'i':
			once_count = getnum();
			break;
		case 'n':
			limit_connection = getnum();
			break;
		case 'l':
			limit_echo = getnum();
			break;
		case 'z':
			limit_echo = getnum();
			Sleep(limit_echo);
			break;
		case 'v':
			view();
			break;
		case 'r':
			reset();
			break;
		case 'd':
			show_info = show_info ? FALSE : TRUE;
			break;
		case 'x':
			break;
		case '?':
			showmenu();
			break;
		default:
		    break;
		}
	}while(wait != 'x');

	for (AEVENTLIST::iterator it = accept_event_list.begin();
		it != accept_event_list.end();++it)(*it)->fini();

	for (CEVENTLIST::iterator it = connect_event_list.begin();
		it != connect_event_list.end();++it)(*it)->fini();

	pool.fini();

	for (AEVENTLIST::iterator it = accept_event_list.begin();
		it != accept_event_list.end();++it)delete *it;

	for (CEVENTLIST::iterator it = connect_event_list.begin();
		it != connect_event_list.end();++it)delete *it;

	WSACleanup();

	return 0;
}
#include "FastCgi.h"
#include "FastCgiSerialize.h"
#include "network/Network.h"
#include "comm/logger.h"

using namespace fish::fastcgi::comm;
using namespace fish::fastcgi::network;

namespace fish{
namespace fastcgi{

static void childProcessExitHandler(int num)   
{   
    //我接受到了SIGCHLD的信号啦   
    int status;   
	waitpid(-1,&status,WNOHANG);   
    WIFEXITED(status);
}  

static void* childThreadHandler(void* fastcgi ){
	FastCgi* temp = (FastCgi*)fastcgi;
	temp->HandleRequest();
	return NULL;
}

FastCgi::FastCgi(){
}

FastCgi::~FastCgi(){
}

void FastCgi::SetNetworkPort( uint16_t dwPort ){
	m_dwPort = dwPort;
	return 0;
}

void FastCgi::SetNetworkThread( uint32_t dwNetworkThread ){
	m_dwNetworkThread = dwNetworkThread;
	return 0;
}

void FastCgi::SetProcess( uint32_t dwProcess ){
	m_dwProcess = dwProcess;
	return 0;
}

void FastCgi::SetCallBack( const CallBackFun& callback ){
	m_callback = callback;
	return 0;
}

int32_t FastCgi::Run(){
	int32_t iRet;
	
	//绑定信号列表
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD,childProcessExitHandler);
	
	//初始化网络
	Network m_network;
	m_network.ListenPort( m_dwPort );
	m_network.SetClientThread( m_dwNetworkThread );
	m_network.SetRequestHandler( m_callback );
		
	//开启多进程
	for( uint32_t i = 1 ; i < m_dwProcess ; ++i ){
		pid = fork();
		if( pid == 0 ){
			break;
		}else if( pid < 0 ){
			Logger::Err("Create Child Process Error!");
			return 1;
		}
	}
	
	//启动网络循环
	iRet = m_network.Run();
	if( iRet != 0 )
		return iRet;
	
	Logger::Info("init success!I am working!");
	
	pthread_exit(NULL);
	return 0;
}


}
}
#include "FastCgi.h"
#include "FastCgiSerialize.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
using namespace fish;
  
static void childProcessExitHandler(int num)   
{   
    //我接受到了SIGCHLD的信号啦   
    int status;   
    int pid = waitpid(-1,&status,WNOHANG);   
    WIFEXITED(status);
}  
static void* childThreadHandler(void* fastcgi ){
	FastCgi* temp = (FastCgi*)fastcgi;
	temp->HandleRequest();
	return NULL;
}
FastCgi::FastCgi(){
	m_serverSocket = -1;
}

FastCgi::~FastCgi(){
	if( m_serverSocket != -1 )
		close( m_serverSocket );
}

int32_t FastCgi::SetNetwork( const std::string& strIP , uint16_t dwPort ){
	m_strIP = strIP;
	m_dwPort = dwPort;
	return 0;
}

int32_t FastCgi::SetProcess( uint32_t dwProcess ){
	m_dwProcess = dwProcess;
	return 0;
}

int32_t FastCgi::SetThreadPerProcess( uint32_t dwThread ){
	m_dwThread = dwThread;
	return 0;
}

int32_t FastCgi::SetCallBack( const CallBackFun& callback ){
	m_callback = callback;
	return 0;
}

int32_t FastCgi::Run(){
	int32_t iRet;
	int pid;
	pthread_t tid;
	
	//绑定端口
	iRet = BindSocket();
	if( iRet != 0 )
		return iRet;
		
	//绑定信号列表
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD,childProcessExitHandler);
	
	//开启多进程
	
	for( uint32_t i = 1 ; i < m_dwProcess ; ++i ){
		pid = fork();
		if( pid == 0 ){
			break;
		}else if( pid < 0 ){
			SetLastMsg("Create Child Process Error!");
			return 1;
		}
	}
	
	//开启多线程
	for ( uint32_t i = 1; i < m_dwThread; i++){
        pthread_create(&tid, NULL, childThreadHandler, this);
		pthread_detach(tid);
	}
	
	childThreadHandler(this);
	return 0;
}

std::string FastCgi::GetLastMsg(){
	return m_strLastMsg;
}

void FastCgi::SetLastMsg( const std::string& strLastMsg ){
	m_strLastMsg = strLastMsg;
}

void FastCgi::LogMsg( const std::string& strMsg ){
	fprintf(stderr,"%s\n",strMsg.c_str());
}

int32_t FastCgi::BindSocket(){
	int iRet;
	int val;
	struct sockaddr_in serverAddIn;
	socklen_t serverAddrLen;
	int socket_type;
	
	memset(&serverAddIn, 0, sizeof(serverAddIn));
	serverAddIn.sin_family = AF_INET;
	serverAddIn.sin_port = htons(m_dwPort);
	serverAddIn.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddrLen = sizeof(serverAddIn);
	socket_type = AF_INET;
		
	m_serverSocket = socket(socket_type, SOCK_STREAM, 0);
	if( m_serverSocket == -1 ){
		SetLastMsg("Create Socket Error!");
		return 1;
	}

	val = 1;
	if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		SetLastMsg("Set Socket ReuseAddr Error!");
		return 1;
	}

	//printf("%d\n",m_dwPort);
	if (-1 == bind(m_serverSocket, (sockaddr*)&serverAddIn, serverAddrLen)) {
		SetLastMsg("Bind Socket Error!");
		return 1;
	}

	if (-1 == listen(m_serverSocket, 5)) {
		SetLastMsg("Listen Socket Error!");
		return 1;
	}
	return 0;
}

int32_t FastCgi::HandleRequest(){
	int clientSocket;
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	FastCgiRequest request;
	FastCgiResponse response;
		
	while( true ){
		clientSocket = accept(m_serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen); 
		if( clientSocket < 0 ){
			LogMsg("accept socket Error");
			continue;
		}
		
		int32_t iRet;
		FastCgiSerialize serialize(clientSocket);
		FCGI_Header* header;
		void* body;
		while( true ){
			iRet = serialize.DeSerializeRequestItem(header,body);
			if( iRet != 0 )
				break;
			else
				printf("Header %d\n",header->type);
		}
		
		m_callback(request,response);
		
		close(clientSocket);
	}
	return 0;
}

int32_t FastCgi::SerializeResponse( const FastCgiResponse& response , std::string& strResponse ){
	return 0;
}

int32_t FastCgi::DeSerializeRequest( const std::string& strRequest  , FastCgiRequest& request ){
	return 0;
}

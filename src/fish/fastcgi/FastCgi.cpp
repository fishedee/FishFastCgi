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
#include <limits.h>
using namespace fish;
  
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
	int32_t iRet;
	
	while( true ){
		clientSocket = accept(m_serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen); 
		if( clientSocket < 0 ){
			LogMsg("accept socket Error");
			continue;
		}
		
		iRet = DeSerializeRequest(clientSocket,request);
		if( iRet != 0 ){
			LogMsg("DeSerializeRequest Error");
			continue;
		}
		
		response.SetRequestId(request.GetRequestId());
		
		m_callback(request,response);
		
		iRet = SerializeResponse(clientSocket,response);
		if( iRet != 0 ){
			LogMsg("DeSerializeRequest Error");
			continue;
		}
		
		close(clientSocket);
	}
	return 0;
}

int32_t FastCgi::SerializeResponse( int clientSocket , const FastCgiResponse& response ){
	FastCgiSerialize serialize(clientSocket);
	FCGI_Header header;
	FCGI_StdOutBody stdOutBody;
	FCGI_StdErrBody stdErrBody;
	FCGI_EndRequestBody endRequestBody;
	int32_t iRet;
	
	header.version = 1;
	header.requestId = response.GetRequestId();
	header.reserved = 0;
	
	if( response.GetOut().size() != 0 ){
		header.type = FCGI_STDOUT;
		
		uint8_t* buffer = (uint8_t*)response.GetOut().c_str();
		uint32_t size = response.GetOut().size();
		while( size != 0 ){
			uint16_t writeSize = SHRT_MAX < size ? SHRT_MAX : size;
			stdOutBody.buffer = buffer;
			stdOutBody.bufferSize = writeSize;
			iRet = serialize.WriteResponseItem( &header , &stdOutBody );
			if( iRet != 0 )
				return iRet;
			
			buffer += writeSize;
			size -= writeSize;
			
		}
		
		stdOutBody.buffer = (uint8_t*)"";
		stdOutBody.bufferSize = 0;
		iRet = serialize.WriteResponseItem( &header , &stdOutBody );
		if( iRet != 0 )
			return iRet;
	}
	
	if( response.GetErr().size() != 0 ){
		header.type = FCGI_STDERR;
		
		uint8_t* buffer = (uint8_t*)response.GetErr().c_str();
		uint32_t size = response.GetErr().size();
		while( size != 0 ){
			uint16_t writeSize = SHRT_MAX < size ? SHRT_MAX : size;
			stdErrBody.buffer = buffer;
			stdErrBody.bufferSize = writeSize;
			iRet = serialize.WriteResponseItem( &header , &stdErrBody );
			if( iRet != 0 )
				return iRet;
			
			buffer += writeSize;
			size -= writeSize;
			
		}
		
		stdErrBody.buffer = (uint8_t*)"";
		stdErrBody.bufferSize = 0;
		iRet = serialize.WriteResponseItem( &header , &stdErrBody );
		if( iRet != 0 )
			return iRet;
	}
	
	header.type = FCGI_END_REQUEST;
	endRequestBody.appStatus = response.GetAppStatus();
	endRequestBody.protocolStatus = response.GetProtocolStatus();
	endRequestBody.reserved[0] = 0;
	endRequestBody.reserved[1] = 0;
	endRequestBody.reserved[2] = 0;
	iRet = serialize.WriteResponseItem( &header , &endRequestBody );
	if( iRet != 0 )
		return iRet;
	return 0;
}

int32_t FastCgi::DeSerializeRequest(  int clientSocket , FastCgiRequest& request ){
	FastCgiSerialize serialize(clientSocket);
	int32_t iRet;
	FCGI_Header* header;
	void* body;
	
	//get header
	iRet = serialize.ReadRequestItem(header,body);
	if( iRet != 0 ){
		LogMsg("ReadRequestItem header Error");
		return iRet;
	}
	if( header->type != FCGI_BEGIN_REQUEST ){
		LogMsg("First Request Item Is Not FCGI_BEGIN_REQUEST");
		return 1;
	}
	request.SetRequestId( header->requestId );
	request.SetFlag( ((FCGI_BeginRequestBody*)body)->flags );
	request.SetRole( ((FCGI_BeginRequestBody*)body)->role );
	iRet = serialize.FreeRequestItem( header , body );
	if( iRet != 0 ){
		LogMsg("FreeRequestItem Error");
		return iRet;
	}
	
	//get params
	while( true ){
		iRet = serialize.ReadRequestItem(header,body);
		if( iRet != 0 ){
			LogMsg("ReadRequestItem params Error");
			return iRet;
		}
		if( header->type != FCGI_PARAMS ){
			LogMsg("First Request Item Is Not FCGI_PARAMS");
			return 1;
		}
		FCGI_ParamsBody* paramBody = (FCGI_ParamsBody*)body;
		if( paramBody == NULL ){
			break;
			serialize.FreeRequestItem( header , body );
		}else{
			for( ; paramBody != NULL ; paramBody = paramBody->next ){
				std::string key(paramBody->param.nameData,paramBody->param.nameData+paramBody->param.nameLength);
				std::string value(paramBody->param.valueData,paramBody->param.valueData+paramBody->param.valueLength);
				request.GetParams()[key] = value;
			}
			serialize.FreeRequestItem( header , body );
		}
	}
	
	//get stdin
	while( true ){
		iRet = serialize.ReadRequestItem(header,body);
		if( iRet != 0 ){
			LogMsg("ReadRequestItem params Error");
			return iRet;
		}
		if( header->type != FCGI_STDIN ){
			LogMsg("First Request Item Is Not FCGI_STDIN");
			return 1;
		}
		FCGI_StdInBody* stdinBody = (FCGI_StdInBody*)body;
		if( stdinBody == NULL || stdinBody->bufferSize == 0 ){
			break;
			serialize.FreeRequestItem( header , body );
		}else{
			request.GetIn().append(stdinBody->buffer,stdinBody->buffer+stdinBody->bufferSize);
			serialize.FreeRequestItem( header , body );
		}
	}
	return 0;
}

#include "ServerSocket.h"
#include "comm/Logger.h"
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
#include <sys/types.h>
#include <fcntl.h>

using namespace fish::fastcgi::comm;

namespace fish{
namespace fastcgi{
namespace network{

ServerSocket::ServerSocket(){
	m_serverSocket = -1;
}
ServerSocket::~ServerSocket(){
}
int32_t ServerSocket::ListenPort( uint16_t dwPort ){
	int val;
	struct sockaddr_in serverAddIn;
	socklen_t serverAddrLen;
	int socket_type;
	int flags;
	
	memset(&serverAddIn, 0, sizeof(serverAddIn));
	serverAddIn.sin_family = AF_INET;
	serverAddIn.sin_port = htons(dwPort);
	serverAddIn.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddrLen = sizeof(serverAddIn);
	socket_type = AF_INET;
		
	m_serverSocket = socket(socket_type, SOCK_STREAM, 0);
	if( m_serverSocket == -1 ){
		Logger::Err("Create Socket Error!");
		return 1;
	}

	flags = fcntl(m_serverSocket, F_GETFL, 0);
	fcntl(m_serverSocket, F_SETFL, flags | O_NONBLOCK);
	
	val = 1;
	if( setsockopt(m_serverSocket, IPPROTO_TCP, TCP_NODELAY,&val,sizeof(val)) < 0 ){
		Logger::Err("Set Socket TCP_NODELAY Error!");
		return 1;
	}
	
	val = 1;
	if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		Logger::Err("Set Socket ReuseAddr Error!");
		return 1;
	}

	if (-1 == bind(m_serverSocket, (sockaddr*)&serverAddIn, serverAddrLen)) {
		Logger::Err("Bind Socket Error!");
		return 1;
	}

	if (-1 == listen(m_serverSocket, 5)) {
		Logger::Err("Listen Socket Error!");
		return 1;
	}
	
	m_isUnixSocket = false;
	return 0;
}
int32_t ServerSocket::ListenUnixSokcet( const std::string& strAddress ){
	int val;
	struct sockaddr_un  serverAddIn;
	socklen_t serverAddrLen;
	int flags;
	
	unlink(strAddress.c_str());
	
	memset(&serverAddIn, 0, sizeof(serverAddIn));
	serverAddIn.sun_family = AF_UNIX;
	strcpy(serverAddIn.sun_path, strAddress.c_str() ); 
	serverAddrLen = sizeof(serverAddIn);
		
	m_serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
	if( m_serverSocket == -1 ){
		Logger::Err("Create Socket Error!");
		return 1;
	}

	flags = fcntl(m_serverSocket, F_GETFL, 0);
	fcntl(m_serverSocket, F_SETFL, flags | O_NONBLOCK);
	
	val = 1;
	if (setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		Logger::Err("Set Socket ReuseAddr Error!");
		return 1;
	}

	if (-1 == bind(m_serverSocket, (sockaddr*)&serverAddIn, serverAddrLen)) {
		Logger::Err("Bind Socket Error!");
		return 1;
	}

	if (-1 == listen(m_serverSocket, 5)) {
		Logger::Err("Listen Socket Error!");
		return 1;
	}
	
	m_isUnixSocket = true;
	return 0;
}
int ServerSocket::GetSocket(){
	return m_serverSocket;
}
bool ServerSocket::IsUnixSocket(){
	return m_isUnixSocket;
}

}
}
}

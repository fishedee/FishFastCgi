#include "ServerSocket.h"
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

namespace fish{
namespace fastcgi{
namespace network{

ServerSocket::ServerSocket(){
	m_socket = -1;
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

	flags == fcntl(m_serverSocket, F_GETFL, 0);
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
	return 0;
}
int ServerSocket::GetSocket(){
	return m_serverSocket;
}

}
}
}

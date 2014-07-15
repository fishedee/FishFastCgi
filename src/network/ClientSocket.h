#ifndef __FAST_CGI_CLIENT_SOCKET_H__
#define __FAST_CGI_CLIENT_SOCKET_H__

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <fcntl.h>

#include <map>
#include <string>
#include "ServerSocket.h"

namespace fish{
namespace fastcgi{
namespace network{

enum{
	CLIENT_SOCKET_ACTIVE_EVENT = 64 ,
	CLIENT_SOCKET_READ_BUFFER = 1024
};

class ClientSocketData{
public:
	std::string	readData;
	std::string writeData;
	int socket;
};

class ClientSocketListener{
public:
	virtual void OnConnected( int socket ) = 0;
	virtual void OnRead( int socket , std::string& request , std::string& response ) = 0;
	virtual void OnClose( int socket ) = 0;
};

class ClientSocket{

public:
	ClientSocket( ServerSocket& serverSocket );
	~ClientSocket();
	
public:
	void SetListener( ClientSocketListener& listener );
	int32_t Run();

private:
	void handleAcceptEvent( int socket );
	void handleReadEvent( ClientSocketData* data  );
	void handleWriteEvent( ClientSocketData* data  );
	void handleCloseEvent( ClientSocketData* data );
	
private:
	int m_epollQueue;
	struct epoll_event m_activeEvents[CLIENT_SOCKET_ACTIVE_EVENT];
	char m_readBuffer[CLIENT_SOCKET_READ_BUFFER];
	ServerSocket& m_serverSocket;
	ClientSocketListener* m_listener;
};

}
}
}
#endif

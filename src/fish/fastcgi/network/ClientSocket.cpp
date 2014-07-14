#include "ClientSocket.h"
#include "Logger.h"

namespace fish{
namespace fastcgi{
namespace network{

ClientSocket::ClientSocket( ServerSocket& serverSocket )
	:m_serverSocket( serverSocket ){
	m_listener = NULL;
}
ClientSocket::~ClientSocket(){
}
void ClientSocket::SetListener( ClientSocketListener& listener ){
	m_listener = &listener;
}
int32_t ClientSocket::Run(){
	struct epoll_event epollEvent;
	int nepollEvent;
	
	//add epoll queue
	m_epollQueue = epoll_create(1024);
	if( m_epollQueue < 0 ){
		Logger::Err("epoll_create error");
		return 1;
	}
	
	//add server socket to epoll
	epollEvent.data.fd  = serverSocket.GetSocket();
	epollEvent.events = EPOLLIN ;
	iRet = epoll_ctl(m_epollQueue, EPOLL_CTL_ADD,serverSocket.GetSocket(),&epollEvent);
	if( iRet < 0 ){
		Logger::Err("epoll_ctl EPOLL_CTL_ADD server socket error");
		return 1;
	}
	
	//go
	while( true ){
		//get active event
		nepollEvent = epoll_wait(m_epollQueue,m_activeEvents,CLIENT_SOCKET_ACTIVE_EVENT,-1);
		if( nepollEvent <= 0 ){
			Logger::Err("epoll_wait error!");
			return 3;
		}
		
		for( i = 0 ; i != nepollEvent ; ++i ){
			if( ( m_activeEvents[i].events & EPOLLIN  ) && 
				m_activeEvents[i].data.fd == serverSocket.GetSocket() ){
				handleAcceptEvent( m_activeEvents[i].data.fd );
			}else if( m_activeEvents[i].events & EPOLLIN ){
				handleReadEvent( m_activeEvents[i].data.fd );
			}else if( m_activeEvents[i].events & EPOLLOUT ){
				handleWriteEvent( m_activeEvents[i].data.fd );
			}
		}
	}
	
	//close
	close(m_epollQueue);
	return 0;
}
void ClientSocket::handleAcceptEvent( int server ){
	struct sockaddr_in connection_addr;
	int sin_size;
	int clientSocket;
	int flags;
	struct epoll_event epollEvent;
	
	//accept connection
	sin_size=sizeof(connection_addr);
	clientSocket =accept(server,(struct sockaddr *)(&connection_addr),&sin_size);
	if( clientSocket ==-1){
		Logger::Err("Accept Error");
		return;
	}
	
	//set non-block
	flags == fcntl(clientSocket, F_GETFL, 0);
	fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
	
	//create socket data
	ClientSocketData* data = new ClientSocketData();
	data->socket = clientSocket;
	
	//add client socket to epoll
	epollEvent.data.ptr  = data;
	epollEvent.events = EPOLLIN ;
	iRet = epoll_ctl(m_epollQueue, EPOLL_CTL_ADD,clientSocket,&epollEvent);
	if( iRet < 0 ){
		Logger::Err("m_epollQueue EPOLL_CTL_ADD Client Socket Error");
		handleCloseEvent( data );
		return;
	}
	
	m_listener->OnConnected(clientSocket);
}
void ClientSocket::handleReadEvent( ClientSocketData* data ){
	int iRet;
	struct epoll_event epollEvent;
	
	while( true ){
		//read data
		iRet = read(data->socket,m_readBuffer,sizeof(m_readBuffer));
		if( iRet == 0 ){
			Logger::Err("read data from socket error");
			handleCloseEvent( data );
			break;
		}
		if( iRet < 0 ){
			if( errno == EINPROGRESS )
				break;
			Logger::Err("read data from socket error");
			handleCloseEvent( data );
			break;
		}
		data->readData.append( m_readBuffer , m_readBuffer + iRet );
	}
	if( data->readData.size() != 0 ){
		//notice program to read
		std::string writeData;
		m_listener->OnRead( data->socket , data->readData ,writeData );
		data->writeData.append( writeData );
		
		//have something to write
		if( data->writeData.size() != 0 ){
			epollEvent.data.ptr  = data;
			epollEvent.events = EPOLLIN | EPOLLOUT ;
			iRet = epoll_ctl(m_epollQueue, EPOLL_CTL_MOD,data->socket,&epollEvent);
			if( iRet < 0 ){
				Logger::Err("m_epollQueue EPOLL_CTL_MOD Client Socket Error");
				handleCloseEvent( data );
				return;
			}		
		}
	}
}
void ClientSocket::handleWriteEvent( ClientSocketData* data ){
	int writeSize = data->writeData.size();
	const char* writeBuffer = data->writeData.c_str(); 
	int iRet;
	struct epoll_event epollEvent;
	
	while( writeSize != 0 ){
		iRet = write(data->socket,writeBuffer,writeSize);
		if( iRet == 0 ){
			Logger::Err("write data to socket error");
			handleCloseEvent( data );
			break;
		}
		if( iRet < 0 ){
			if( errno == EINPROGRESS )
				break;
			Logger::Err("write data to socket error");
			handleCloseEvent( data );
			break;
		}
		writeBuffer += writeSize;
		writeSize -= iRet;
	}
	data->writeData.erase( 0 , data->writeData.size() - writeSize );
	if( data->writeData.size() == 0 ){
		//have nothing to write
		epollEvent.data.ptr  = data;
		epollEvent.events = EPOLLIN ;
		iRet = epoll_ctl(m_epollQueue, EPOLL_CTL_MOD,data->socket,&epollEvent);
		if( iRet < 0 ){
			Logger::Err("m_epollQueue EPOLL_CTL_MOD Client Socket Error");
			handleCloseEvent( data );
			return;
		}
	}
}
void ClientSocket::handleCloseEvent( ClientSocketData* data ){
	struct epoll_event epollEvent;
	
	//notice listener to close
	m_listener->OnClose( data->socket );
	
	//remove socket from epoll
	epollEvent.data.ptr  = data;
	epollEvent.events = EPOLLIN | EPOLLOUT ;
	epoll_ctl(m_epollQueue, EPOLL_CTL_DEL,data->socket,&epollEvent);
	
	//remove socket
	close(data->socket);
	
	//remove data
	delete data;
}

	
}
}
}
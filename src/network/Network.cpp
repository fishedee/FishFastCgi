#include "Network.h"
#include "comm/Logger.h"
#include <pthread.h>

using namespace fish::fastcgi::comm;

namespace fish{
namespace fastcgi{
namespace network{

Network::Network(){
}
Network::~Network(){
}
void Network::ListenPort( uint16_t dwPort ){
	m_serverSocket.ListenPort( dwPort );
}
void Network::ListenUnixAddress( const std::string& strAddress ){
	m_serverSocket.ListenUnixSokcet( strAddress );
}
void Network::SetClientThread( uint32_t dwThread ){
	m_dwThread = dwThread;
}
void Network::SetListener( NetworkListener& listener ){
	m_listener = &listener;
}
int32_t Network::Run(){
	int iRet;
	pthread_t tid;
	
	//init client Sockets
	for( uint32_t i = 0 ; i != m_dwThread ; ++i ){
	
		ClientSocket clientSocket( m_serverSocket );
		FastCgiSocket fastcgiSocket( clientSocket );
		
		fastcgiSocket.SetRequestListener(*this);
		
		m_vecClientSockets.push_back( fastcgiSocket );
	}
	
	//run client socket
	for( uint32_t i = 1 ; i != m_dwThread ; ++i ){
		iRet = pthread_create( &tid , NULL , &Network::ThreadHelper , &m_vecClientSockets[i] );
		if( iRet != 0 ){
			Logger::Err("pthread_create error!");
			return 1;
		}

		iRet = pthread_detach( tid );
		if( iRet != 0 ){
			Logger::Err("pthread_detach error!");
			return 2;
		}
	}
	
	ThreadHelper( &m_vecClientSockets[0] );
	return 0;
}
void Network::OnRequest( const FastCgiRequest&request , FastCgiResponse& response ){
	m_listener->OnRequest( request , response );
}
void* Network::ThreadHelper( void* arg ){
	FastCgiSocket* temp = (FastCgiSocket*)arg;
	temp->Run();
	return NULL;
}
	

}
}
}

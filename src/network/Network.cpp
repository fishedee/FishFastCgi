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
	for( uint32_t i = 0 ; i != m_dwThread ; ++i ){
		ClientSocket* clientSocket = m_vecClientSockets[i]->GetClientSocket();
		FastCgiSocket* fastcgiSocket = m_vecClientSockets[i];
		Logger::Debug("ClientSocket Delete "+std::to_string((int64_t)(void*)clientSocket));
		Logger::Debug("ClientSocket Delete "+std::to_string((int64_t)(void*)fastcgiSocket));
		delete fastcgiSocket;
		delete clientSocket;
	}
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
	
		ClientSocket* clientSocket = new ClientSocket( m_serverSocket );
		FastCgiSocket* fastcgiSocket = new FastCgiSocket( *clientSocket );
		
		Logger::Debug("ClientSocket New "+std::to_string((int64_t)(void*)clientSocket));
		Logger::Debug("ClientSocket New "+std::to_string((int64_t)(void*)fastcgiSocket));
		fastcgiSocket->SetRequestListener(*this);
		
		m_vecClientSockets.push_back( fastcgiSocket );
	}
	
	//run client socket
	for( uint32_t i = 0 ; i != m_dwThread ; ++i ){
		iRet = pthread_create( &tid , NULL , &Network::ThreadHelper , m_vecClientSockets[i] );
		if( iRet != 0 ){
			Logger::Err("pthread_create error!");
			return 1;
		}

		m_vecThread.push_back( tid );
	}
	
	return 0;
}
int32_t Network::Wait(){
	for( uint32_t i = 0 ; i != m_dwThread ; ++i )
		pthread_join( m_vecThread[i] , NULL );
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

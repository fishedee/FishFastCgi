#include "Network.h"

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
void Network::SetClientThread( uint32_t dwThread ){
	m_dwThread = dwThread;
}
void Network::SetRequestHandler( const RequestHandler& requestHandler ){
	m_requestHandler = requestHandler;
}
int32_t Network::Run(){
	//init client Sockets
	for( uint32_t i = 0 ; i != m_dwThread ; ++i ){
	
		ClientSocket clientSocket( m_serverSocket );
		FastCgiSocket fastcgiSocket( clientSocket );
		
		fastcgiSocket.SetRequestHandler(m_requestHandler);
		
		m_vecClientSockets.push_back( fastcgiSocket );
	}
	
	//run client socket
	for( uint32_t i = 0 ; i != m_dwThread ; ++i )
		m_vecClientSockets[i].Run();
		
	return 0;
}

}
}
}

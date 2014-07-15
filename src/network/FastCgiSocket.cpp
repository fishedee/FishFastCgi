#include "FastCgiSocket.h"
#include "comm/Logger.h"

using namespace fish::fastcgi::comm;

namespace fish{
namespace fastcgi{
namespace network{

FastCgiSocket::FastCgiSocket( ClientSocket& clientSocket )
	:m_clientSocket(clientSocket){
	clientSocket.SetListener( this );
	
}
FastCgiSocket::~FastCgiSocket(){
}
void FastCgiSocket::SetRequestListener( FastCgiSocketListener& listener ){
	m_listener = &listener;
}
int32_t FastCgiSocket::Run(){
	return clientSocket.Run();
}
void FastCgiSocket::OnConnected( int socket ){
}
void FastCgiSocket::OnRead( int socket , std::string& request , std::string& response ){
	uint32_t nReadSize = 0;
	uint8_t* headerBuffer = (uint8_t*)request.c_str();
	
	//read data
	while( true ){
	
		if( request.size() < 8 )
			return;
		
		uint16_t contentLength = ( ((uint8_t)request[4]) << 8 ) + (uint8_t)request[5];
		uint8_t paddingLength = (uint8_t)request[6];
		
		if( request.size() < 8 + contentLength + paddingLength )
			return ;
			
		ReadHeader(socket,headerBuffer , response);
		
		nReadSize += 8 + contentLength + paddingLength;
		headerBuffer += 8 + contentLength + paddingLength;
	}
	
	//remove data what have readed
	if( nReadSize != 0 )
		request.erase(0,nReadSize);
	
}
void FastCgiSocket::OnClose( int socket ){
	for( std::map<uint16_t,std::list<FCGI_Header*> >::iterator it = m_mapHeaderList[socket].begin();
		it != m_mapHeaderList[socket].end() ; ++it ){
		CloseRequest( socket , it->first );
	}
	m_mapHeaderList.remove( socket );
}
void FastCgiSocket::ReadHeader( int socket , const char* buffer , std::string& response ){
	FCGI_Header* header = new FCGI_Header();
	
	//read header
	header->version = buffer[0];
	header->type = buffer[1];
	header->requestId = (buffer[2] << 8) + buffer[3];
	header->contentLength = (buffer[4] << 8) + buffer[5];
	header->paddingLength = buffer[6];
	header->reserved = buffer[7];
	
	//read content
	header->content = new uint8_t[header->contentLength];
	memcpy( header->content , buffer + 8 , header->contentLength );
	
	if( header->type == FCGI_GET_VALUES ){
		ReadGetValues( socket , header );
		delete[] header->content;
		delete header;
	}else{
		m_mapHeaderList[socket][requestId].push_back( header );
		//on read stdin finish
		if( header->type == FCGI_STDIN && header->contentLength == 0 ){
			ReadStdInFinish( socket , header->requestId , response );
		}
	}
}
void FastCgiSocket::ReadStdInFinish( int socket , uint16_t requestId , std::string& strResponse ){
	FastCgiRequest request;
	FastCgiResponse response;
	int32_t iRet;
	
	iRet = m_protocol.DeSerializeRequest( m_mapHeaderList[socket][requestId] , request );
	if( iRet != 0 )
		return;
	
	response.SetRequestId( request.GetRequestId() );
	m_listener->OnRequest( request , response );
	
	iRet = m_protocol.SerializeResponse( response , strResponse );
	if( iRet != 0 )
		return;
		
	CloseRequest(socket,requestId);
	
	return;
}
void FastCgiSocket:ReadGetValues( int socket , FCGI_Header* header ){
	FastCgiRequest request;
	
	iRet = m_protocol.DeSerializeGetValues( header , request );
	if( iRet != 0 )
		return;
		
	return;
}
void FastCgiSocket::CloseRequest( int socket , uint16_t requestId ){
	for( std::list<FCGI_Header*>::iterator it = m_mapHeaderList[socket][requestId].begin();
		it != m_mapHeaderList[socket][requestId].end() ;++it ){
		delete[] (*it)->content;
		delete (*it);
	}
}
	
}
}
}
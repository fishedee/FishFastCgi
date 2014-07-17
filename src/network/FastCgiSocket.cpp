#include "FastCgiSocket.h"
#include "comm/Logger.h"
#include <string.h>

using namespace fish::fastcgi::comm;

namespace fish{
namespace fastcgi{
namespace network{

FastCgiSocket::FastCgiSocket( ClientSocket& clientSocket )
	:m_clientSocket(clientSocket){
	m_clientSocket.SetListener( *this );
	
}
FastCgiSocket::~FastCgiSocket(){
}
void FastCgiSocket::SetRequestListener( FastCgiSocketListener& listener ){
	m_listener = &listener;
}
int32_t FastCgiSocket::Run(){
	Logger::Debug("FastCgiSocket Run!");
	return m_clientSocket.Run();
}
ClientSocketData* FastCgiSocket::OnConnected(){
	FastCgiSocketData* data = new FastCgiSocketData();
	return data;
}
void FastCgiSocket::OnRead( ClientSocketData* data ){
	uint32_t nReadSize = 0;
	uint32_t nTotalSize = data->readData.size();
	const char* headerBuffer = data->readData.c_str();
	
	//read data
	while( true ){
	
		if( nTotalSize < 8 )
			break;
		
		uint16_t contentLength = ( ((uint8_t)headerBuffer[4]) << 8 ) + (uint8_t)headerBuffer[5];
		uint8_t paddingLength = (uint8_t)headerBuffer[6];
		
		if( nTotalSize < (uint32_t)(8 + contentLength + paddingLength) )
			break ;
			
		ReadSingleRequest(headerBuffer , (FastCgiSocketData*)data );
		
		nReadSize += 8 + contentLength + paddingLength;
		headerBuffer += 8 + contentLength + paddingLength;
		nTotalSize -= 8 + contentLength + paddingLength;
	}
	
	//remove data what have readed
	if( nReadSize != 0 )
		data->readData.erase(0,nReadSize);
	
}
void FastCgiSocket::OnClose( ClientSocketData* data  ){
	delete data;
}
void FastCgiSocket::ReadSingleRequest( const char* buffer , FastCgiSocketData* data ){
	int32_t iRet;
	
	iRet = m_protocol.DeSerializeRequest( buffer , data->request );
	if( iRet != 0 )
		return;
		
	data->response.SetRequestId( data->request.GetRequestId() );
	
	m_listener->OnRequest( data->request , data->response );
	
	iRet = m_protocol.SerializeResponse( data->response , data->writeData );
	if( iRet != 0 )
		return;
		
	data->request.Clear();
	data->response.Clear();
}
	
}
}
}
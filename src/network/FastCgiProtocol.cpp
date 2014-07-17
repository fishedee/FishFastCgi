#include "FastCgiProtocol.h"
#include "comm/Logger.h"
#include <limits.h>

using namespace fish::fastcgi::comm;

namespace fish{
namespace fastcgi{
namespace network{

FastCgiProtocol::FastCgiProtocol(){
}

FastCgiProtocol::~FastCgiProtocol(){
}

int32_t FastCgiProtocol::DeSerializeRequest( const char* buffer, FastCgiRequest& request ){
	FCGI_Header header;
	
	header.version = buffer[0];
	header.type = buffer[1];
	header.requestId = (buffer[2] << 8) + buffer[3];
	header.contentLength = (buffer[4] << 8) + buffer[5];
	header.paddingLength = buffer[6];
	header.reserved = buffer[7];
	header.content = (uint8_t*)&buffer[8];
	
	if( header.type == FCGI_STDIN && header.contentLength == 0  )
		return 0;
		
	if( header.type == FCGI_BEGIN_REQUEST )
		DeSerializeBeginRequest( &header , request );
	else if( header.type == FCGI_STDIN )
		DeSerializeStdIn( &header , request );
	else if( header.type == FCGI_DATA )
		DeSerializeData( &header ,request );
	else if( header.type == FCGI_PARAMS )
		DeSerializeParams( &header ,request );
	else
		Logger::Err("UnKnown header type " + std::to_string(header.type) );
	
	return 1;
}

int32_t FastCgiProtocol::SerializeResponse( const FastCgiResponse& response , std::string& strResponse ){
	int32_t iRet;
		
	if( response.GetOut().size() != 0 ){
		iRet = SerializeData( response.GetOut().c_str() , response.GetOut().size() ,FCGI_STDOUT, response.GetRequestId() , strResponse );
		if( iRet != 0 )
			return iRet;
	}
	
	if( response.GetErr().size() != 0 ){
		iRet = SerializeData( response.GetErr().c_str()  , response.GetErr().size() ,FCGI_STDERR, response.GetRequestId() ,strResponse );
		if( iRet != 0 )
			return iRet;
	}
		
	if( response.GetData().size() != 0 ){
		iRet = SerializeData( response.GetData().c_str()  , response.GetData().size() ,FCGI_DATA, response.GetRequestId() ,strResponse );
		if( iRet != 0 )
			return iRet;
	}
		
	iRet = SerializeEndRequest( response , strResponse );
	if( iRet != 0 )
		return iRet;
	
	return 0;
}

int32_t FastCgiProtocol::DeSerializeGetValues( const FCGI_Header* header , FastCgiRequest& request ){
	int32_t iRet;
	
	if( header->type != FCGI_GET_VALUES ){
		Logger::Err("It is not get values type");
		return 1;
	}
		
	iRet = DeSerializeParams( header , request );
	if( iRet != 0 )
		return iRet;
		
	return 0;
}

int32_t FastCgiProtocol::SerializeGetValuesResult( uint16_t requestId , const std::map<std::string,std::string>& response , std::string& strResponse ){
	std::string strBuffer;
	int32_t iRet;
	
	for( std::map<std::string,std::string>::const_iterator it = response.begin() ; 
		it != response.end() ; ++it ){
		strBuffer += (char)it->first.size();
		strBuffer += (char)it->second.size();
		strBuffer += it->first.c_str();
		strBuffer += it->second.c_str();
	}

	FCGI_Header header;
	header.version = 1;
	header.type = FCGI_GET_VALUES_RESULT;
	header.requestId = requestId;
	header.contentLength = strBuffer.size();
	header.reserved = 0;
	iRet = SerializeHeader( header , strResponse );
	if( iRet != 0 )
		return iRet;
		
	strResponse.append( strBuffer );
	
	iRet = SerializePadding( header , strResponse );
	if( iRet != 0 )
		return iRet;
	
	return 0;
}

int32_t FastCgiProtocol::DeSerializeParams( const FCGI_Header* header, FastCgiRequest& request ){
	uint8_t nameLengthB3;  /* nameLengthB3  >> 7 == 1 */
	uint8_t nameLengthB2;
	uint8_t nameLengthB1;
	uint8_t nameLengthB0;
	uint8_t valueLengthB3; /* valueLengthB3 >> 7 == 1 */
	uint8_t valueLengthB2;
	uint8_t valueLengthB1;
	uint8_t valueLengthB0;
	uint32_t nameLength;
	uint32_t valueLength;
	uint16_t contentPos = 0;
	
	while( contentPos < header->contentLength ){
		//读取key的长度
		nameLengthB3 = header->content[contentPos++];
		if( nameLengthB3 >> 7 == 1 ){
			nameLengthB2 = header->content[contentPos++];
			nameLengthB1 = header->content[contentPos++];
			nameLengthB0 = header->content[contentPos++];
		}else{
			nameLengthB0 = nameLengthB3;
			nameLengthB3 = nameLengthB2 = nameLengthB1 = 0;
		}
		
		//读取value的长度
		valueLengthB3 = header->content[contentPos++];
		if( valueLengthB3 >> 7 == 1 ){
			valueLengthB2 = header->content[contentPos++];
			valueLengthB1 = header->content[contentPos++];
			valueLengthB0 = header->content[contentPos++];
		}else{
			valueLengthB0 = valueLengthB3;
			valueLengthB3 = valueLengthB2 = valueLengthB1 = 0;
		}
		
		//读取key,value数据
		nameLength = ((nameLengthB3 & 0x7f) << 24) + (nameLengthB2 << 16) + (nameLengthB1 << 8) + nameLengthB0;
		valueLength = ((valueLengthB3 & 0x7f) << 24) + (valueLengthB2 << 16) + (valueLengthB1 << 8) + valueLengthB0;
		
		std::string key(header->content + contentPos , header->content + contentPos + nameLength );
		std::string value(header->content + contentPos + nameLength , header->content + contentPos + nameLength + valueLength );
		request.GetParams()[key] = value;
		
		contentPos += nameLength + valueLength;
	}
	return 0;
}

int32_t FastCgiProtocol::DeSerializeStdIn( const FCGI_Header* headerList, FastCgiRequest& request ){
	request.SetIn( std::string(headerList->content,headerList->content+headerList->contentLength) );
	return 0;
}

int32_t FastCgiProtocol::DeSerializeData( const FCGI_Header* headerList, FastCgiRequest& request ){
	request.SetData( std::string(headerList->content,headerList->content+headerList->contentLength) );
	return 0;
}

int32_t FastCgiProtocol::DeSerializeBeginRequest( const FCGI_Header* header, FastCgiRequest& request ){
	uint16_t role = ( header->content[0] << 8 ) + header->content[1];
	uint8_t flags = header->content[2];
	
	request.SetRole( role );
	request.SetFlag( flags );
	request.SetRequestId( header->requestId );
	return 0;
}

int32_t FastCgiProtocol::SerializeEndRequest( const FastCgiResponse& response , std::string& strResponse ){
	int32_t iRet;
	
	FCGI_Header header;
	header.version = 1;
	header.type = FCGI_END_REQUEST;
	header.requestId = response.GetRequestId();
	header.contentLength = 8;
	header.reserved = 0;
	iRet = SerializeHeader( header , strResponse );
	if( iRet != 0 )
		return iRet;
		
	char buffer[8] = {0};
	buffer[0] = ( response.GetAppStatus() >> 24 ) & 0xFF;
	buffer[1] = ( response.GetAppStatus() >> 16 ) & 0xFF;
	buffer[2] = ( response.GetAppStatus() >> 8 ) & 0xFF;
	buffer[3] = ( response.GetAppStatus() ) & 0xFF;
	buffer[4] = response.GetProtocolStatus();
	strResponse.append( buffer , buffer + sizeof(buffer) );
	
	iRet = SerializePadding( header , strResponse );
	if( iRet != 0 )
		return iRet;
	
	return 0;
}
int32_t FastCgiProtocol::SerializeData( const char* data ,uint32_t size , uint8_t type , uint16_t requestId , std::string& strResponse ){
	int32_t iRet;
	uint8_t* buffer = (uint8_t*)data;
	uint32_t length = size;
	
	while( length != 0 ){
		uint16_t writeSize = length < SHRT_MAX ? length : SHRT_MAX;
		
		FCGI_Header header;
		header.version = 1;
		header.type = type;
		header.requestId = requestId;
		header.contentLength = writeSize;
		header.reserved = 0;
		iRet = SerializeHeader( header , strResponse );
		if( iRet != 0 )
			return iRet;
			
		strResponse.append( buffer , buffer + writeSize );
		
		iRet = SerializePadding( header , strResponse );
		if( iRet != 0 )
			return iRet;
		
		buffer += writeSize;
		length -= writeSize;
	}
	
	FCGI_Header header;
	header.version = 1;
	header.type = type;
	header.requestId = requestId;
	header.contentLength = 0;
	header.reserved = 0;
	iRet = SerializeHeader( header , strResponse );
	if( iRet != 0 )
		return iRet;
	
	return 0;
}

int32_t FastCgiProtocol::SerializeHeader( FCGI_Header& header , std::string& strResponse ){
	char buffer[8];
	
	header.paddingLength = ( 8 - header.contentLength%8 );
	if( header.paddingLength == 8 )
		header.paddingLength = 0;
		
	buffer[0] = header.version;
	buffer[1] = header.type;
	buffer[2] = (header.requestId >> 8 ) & 0xFF;
	buffer[3] = (header.requestId ) & 0xFF;
	buffer[4] = (header.contentLength >> 8 ) & 0xFF;
	buffer[5] = (header.contentLength ) & 0xFF;
	buffer[6] = header.paddingLength;
	buffer[7] = header.reserved;
	
	strResponse.append( buffer , buffer + sizeof(buffer) );
	return 0;
}

int32_t FastCgiProtocol::SerializePadding( FCGI_Header& header , std::string& strResponse ){
	char buffer[8] = {0};
	
	strResponse.append( buffer , buffer + header.paddingLength );
	return 0;
}

}
}
}
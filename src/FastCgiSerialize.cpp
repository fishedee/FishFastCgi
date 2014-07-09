#include "FastCgiSerialize.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string>

using namespace fish;
using namespace std;

FastCgiSerialize::FastCgiSerialize( int socket ){
	m_socket = socket;
	m_readBufferPos = m_writeBufferPos = 0;
	m_readBufferSize = m_writeBufferSize = 0;
}
FastCgiSerialize::~FastCgiSerialize(){
}
int32_t FastCgiSerialize::DeSerializeRequestItem( FCGI_Header*& header , void*& body ){
	int32_t iRet;
	//反序列化头部开头
	iRet = DeSerializeHeader(header);
	if( iRet != 0 ){
		LogMsg("DeSerializeHeader Error!");
		return iRet;
	}
	
	//反序列内容
	if( header->type == FCGI_BEGIN_REQUEST ){
		iRet = DeSerializeBeginRequestBody( header , body );
	}else if( header->type == FCGI_PARAMS ){
		iRet = DeSerializeParamsBody( header , body );
	}else if( header->type == FCGI_STDIN ){
		iRet = DeSerializeStdinBody( header ,body );
	}else if( header->type == FCGI_DATA ){
		iRet = DeSerializeStdinBody( header ,body );
	}else if( header->type == FCGI_GET_VALUES ){
		iRet = DeSerializeGetValuesBody( header ,body );
	}else{
		iRet = DeSerializeUnknownBody( header ,body );
	}
	if( iRet != 0 ){
		LogMsg("DeSerializeBody Error! " + to_string(header->type));
		return iRet;
	}
	
	return 0;
}
int32_t FastCgiSerialize::FreeRequestItem( FCGI_Header*& header , void*& body ){
	header = NULL;
	body = NULL;
	return 0;
}
void FastCgiSerialize::LogMsg( const std::string& strMsg ){
	fprintf(stderr,"%s\n",strMsg.c_str());
}
int32_t FastCgiSerialize::Read( uint8_t* buffer , int size ){
	//读取socket数据到buffer中
	while( size != 0 ){
		int bufferReleaseCount = m_readBufferSize - m_readBufferPos;
		if(  bufferReleaseCount > 0 ){
			//if have release buffer
			int readBufferCount = bufferReleaseCount < size ? bufferReleaseCount : size;
			memcpy( buffer , m_readBuffer + m_readBufferPos , readBufferCount );
			size -= readBufferCount;
			buffer += readBufferCount;
			m_readBufferPos += readBufferCount;
		}else{
			//if have not release buffer
			m_readBufferSize = read( m_socket , m_readBuffer , FCGI_MAX_READ_BUFFER_SIZE );
			if( m_readBufferSize <= 0 ){
				LogMsg("read data Error!");
				return 1;
			}
			m_readBufferPos = 0;
			
		}
	}
	return 0;
}
int32_t FastCgiSerialize::ReadContent( FCGI_Header* header , uint8_t* buffer , int size ){
	uint16_t releaseCount = header->contentLength - header->contentPos;
	if( releaseCount < size ){
		LogMsg("header content length:" + to_string(header->contentLength)
			+ " header content pos:" + to_string(header->contentPos)
			+ " size:" + to_string(size) );
		return 1;
	}
	memcpy( buffer , header->content + header->contentPos, size );
	header->contentPos += size;
	return 0;
}
int32_t FastCgiSerialize::ReadPadding( FCGI_Header* header , uint8_t* buffer , int size ){
	uint16_t releaseCount = header->paddingLength - header->paddingPos;
	if( releaseCount < size )
		return 1;
	memcpy( buffer , header->padding , size );
	header->paddingPos += size;
	return 0;
}
int32_t FastCgiSerialize::DeSerializeHeader( FCGI_Header*& header ){
	int32_t iRet;
	uint8_t buffer[8];
	
	header = (FCGI_Header*)malloc(sizeof(FCGI_Header));
	iRet = Read( buffer , sizeof(buffer) );
	if( iRet != 0 )
		return iRet;
	for( uint32_t i = 0 ; i != 8 ; ++i )
		printf("%x\n",buffer[i]);
		
	header->version = buffer[0];
	header->type = buffer[1];
	header->requestId = (buffer[2] << 8) + buffer[3];
	header->contentLength = (buffer[4] << 8) + buffer[5];
	header->paddingLength = buffer[6];
	header->reserved = buffer[7];
	header->content = (uint8_t*)malloc(header->contentLength);
	header->padding = (uint8_t*)malloc(header->paddingLength);
	header->contentPos = 0;
	header->paddingPos = 0;
	iRet = Read( header->content , header->contentLength );
	if( iRet != 0 )
		return iRet;
	iRet = Read( header->padding , header->paddingLength );
	if( iRet != 0 )
		return iRet;
	printf("XX %d\n",buffer[4] );
	printf("XX %d\n",buffer[5] );
	printf("XX %d\n",(buffer[4] << 8) + buffer[5]);
	return 0;
}
int32_t FastCgiSerialize::DeSerializeBeginRequestBody( FCGI_Header* header , void*& resultBody ){
	FCGI_BeginRequestBody* body;
	int32_t iRet;
	uint8_t buffer[8];
	
	body = (FCGI_BeginRequestBody*)malloc(sizeof(FCGI_BeginRequestBody));
	iRet = ReadContent( header , buffer , sizeof(buffer) );
	if( iRet != 0 )
		return iRet;
	body->role = (buffer[0] << 8) + buffer[1];
	body->flags = buffer[2];
	memcpy(body->reserved,&buffer[3],5);
	
	resultBody = body;
	
	return 0;
}
int32_t FastCgiSerialize::DeSerializeParamsBody( FCGI_Header* header , void*& resultBody ){
	FCGI_ParamsBody* body = NULL;
	FCGI_ParamsBody* prevBody;
	int32_t iRet;
	unsigned char nameLengthB3;  /* nameLengthB3  >> 7 == 1 */
	unsigned char nameLengthB2;
	unsigned char nameLengthB1;
	unsigned char nameLengthB0;
	unsigned char valueLengthB3; /* valueLengthB3 >> 7 == 1 */
	unsigned char valueLengthB2;
	unsigned char valueLengthB1;
	unsigned char valueLengthB0;
	
	while( header->contentPos < header->contentLength ){
		prevBody = body;
		body = (FCGI_ParamsBody*)malloc(sizeof(FCGI_ParamsBody));
		body->next = prevBody;
		//读取key的长度
		iRet = ReadContent( header , &nameLengthB3 , 1 );
		if( iRet != 0 )
			return iRet;
		if( nameLengthB3 >> 7 == 1 ){
			iRet = ReadContent( header , &nameLengthB2 , 1 );
			if( iRet != 0 )
				return iRet;
			iRet = ReadContent( header , &nameLengthB1 , 1 );
			if( iRet != 0 )
				return iRet;
			iRet = ReadContent( header , &nameLengthB0 , 1 );
			if( iRet != 0 )
				return iRet;
		}else{
			nameLengthB0 = nameLengthB3;
			nameLengthB3 = nameLengthB2 = nameLengthB1 = 0;
		}
		//读取value的长度
		iRet = ReadContent( header , &valueLengthB3 , 1 );
		if( iRet != 0 )
			return iRet;
		if( valueLengthB3 >> 7 == 1 ){
			iRet = ReadContent( header , &valueLengthB2 , 1 );
			if( iRet != 0 )
				return iRet;
			iRet = ReadContent( header , &valueLengthB1 , 1 );
			if( iRet != 0 )
				return iRet;
			iRet = ReadContent( header , &valueLengthB0 , 1 );
			if( iRet != 0 )
				return iRet;
		}else{
			valueLengthB0 = valueLengthB3;
			valueLengthB3 = valueLengthB2 = valueLengthB1 = 0;
		}
		//读取key数据
		body->param.nameLength = ((nameLengthB3 & 0x7f) << 24) + (nameLengthB2 << 16) + (nameLengthB1 << 8) + nameLengthB0;
		body->param.nameData = (unsigned char*)malloc(body->param.nameLength);
		iRet = ReadContent( header , body->param.nameData, body->param.nameLength );
		if( iRet != 0 )
			return iRet;
		//读取value数据
		body->param.valueLength = ((valueLengthB3 & 0x7f) << 24) + (valueLengthB2 << 16) + (valueLengthB1 << 8) + valueLengthB0;
		body->param.valueData = (unsigned char*)malloc(body->param.valueLength);
		iRet = ReadContent( header , body->param.valueData, body->param.valueLength );
		if( iRet != 0 )
			return iRet;
		
		printf("%s %s\n",body->param.nameData,body->param.valueData);
	}
	resultBody = body;
	
	return 0;
}
int32_t FastCgiSerialize::DeSerializeStdinBody( FCGI_Header* header , void*& resultBody ){
	FCGI_StdInBody* body;
	int32_t iRet;
	
	body = (FCGI_StdInBody*)malloc(sizeof(FCGI_StdInBody));
	body->buffer = (unsigned char*)malloc(header->contentLength);
	body->bufferSize = header->contentLength;
	iRet = ReadContent( header, body->buffer , body->bufferSize );
	if( iRet != 0 )
		return iRet;
	resultBody = body;
	
	printf("XX %s\n",body->buffer );
	return 0;
	
}
int32_t FastCgiSerialize::DeSerializeDataBody( FCGI_Header* header , void*& resultBody ){
	return DeSerializeStdinBody( header , resultBody );
}
int32_t FastCgiSerialize::DeSerializeGetValuesBody( FCGI_Header* header , void*& resultBody ){
	return DeSerializeParamsBody( header , resultBody );
}
int32_t FastCgiSerialize::DeSerializeUnknownBody( FCGI_Header* header , void*& resultBody ){
	FCGI_UnknownTypeBody* body;
	int32_t iRet;
	
	body = (FCGI_UnknownTypeBody*)malloc(sizeof(FCGI_UnknownTypeBody));
	iRet = ReadContent( header , (uint8_t*)body , sizeof(FCGI_UnknownTypeBody) );

	resultBody = body;
	return 0;
}
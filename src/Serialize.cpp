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
	m_readBufferSize = m_readBufferPos  = 0;
	m_writeBufferSize = FCGI_MAX_WRITE_BUFFER_SIZE;
	m_writeBufferPos = 0;
}
FastCgiSerialize::~FastCgiSerialize(){
	WriteFlush();
}
int32_t FastCgiSerialize::WriteResponseItem( FCGI_Header* header , void* body ){
	int32_t iRet;
	if( header->type == FCGI_STDOUT ){
		iRet = SerializeStdOut( header , body );
	}else if( header->type == FCGI_STDERR ){
		iRet = SerializeStdErr( header , body );
	}else if( header->type == FCGI_DATA ){
		iRet = SerializeData( header ,body );
	}else if( header->type == FCGI_END_REQUEST ){
		iRet = SerializeEndRequest( header ,body );
	}else{
		LogMsg("Unknown header type!");
		return 1;
	}
	if( iRet != 0 ){
		LogMsg("SerializeItem Error! " + to_string(header->type));
		return iRet;
	}
	return 0;
}
int32_t FastCgiSerialize::FinishResponseItem(){
	return WriteFlush();
}
int32_t FastCgiSerialize::ReadRequestItem( FCGI_Header*& header , void*& body ){
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
	if( header->type == FCGI_BEGIN_REQUEST ){
		free(body);
	}else if( header->type == FCGI_PARAMS ){
		FCGI_ParamsBody* paramBody = (FCGI_ParamsBody*)body;
		FCGI_ParamsBody* nextParamBody;
		for( ; paramBody != NULL ; paramBody = nextParamBody ){
			nextParamBody = paramBody->next;
			free(paramBody->param.nameData);
			free(paramBody->param.valueData);
			free(paramBody);
		}
	}else if( header->type == FCGI_STDIN ){
		FCGI_StdInBody* stdinBody = (FCGI_StdInBody*)body;
		free(stdinBody->buffer);
		free(stdinBody);
	}else if( header->type == FCGI_DATA ){
		FCGI_DataBody* dataBody = (FCGI_DataBody*)body;
		free(dataBody->buffer);
		free(dataBody);
	}else if( header->type == FCGI_GET_VALUES ){
		FCGI_GetValuesBody* paramBody = (FCGI_GetValuesBody*)body;
		FCGI_GetValuesBody* nextParamBody;
		for( ; paramBody != NULL ; paramBody = nextParamBody ){
			nextParamBody = paramBody->next;
			free(paramBody->param.nameData);
			free(paramBody->param.valueData);
			free(paramBody);
		}
	}else{
		free(body);
	}
	free(header);
	header = NULL;
	body = NULL;
	return 0;
}
void FastCgiSerialize::LogMsg( const std::string& strMsg ){
	fprintf(stderr,"%s\n",strMsg.c_str());
}
int32_t FastCgiSerialize::Write( uint8_t* buffer , int size ){
	int32_t iRet;
	while( size != 0 ){
		if( m_writeBufferSize == m_writeBufferPos ){
			//write buffer is full
			iRet = WriteFlush();
			if( iRet != 0 )
				return iRet;
		}else{
			//write buffer isnot full
			int bufferReleaseCount = m_writeBufferSize - m_writeBufferPos;
			int writeBufferCount = bufferReleaseCount < size ? bufferReleaseCount : size;
			memcpy( m_writeBuffer + m_writeBufferPos , buffer , writeBufferCount );
			size -= writeBufferCount;
			buffer += writeBufferCount;
			m_writeBufferPos += writeBufferCount;
		}
	}
	return 0;
}
int32_t FastCgiSerialize::WriteFlush(){
	uint8_t* writeBuffer = m_writeBuffer;
	int32_t writeBufferCount = 0;
	while( writeBuffer != m_writeBuffer + m_writeBufferPos ){
		writeBufferCount = write( m_socket , writeBuffer , m_writeBuffer + m_writeBufferPos -  writeBuffer );
		if( writeBufferCount <= 0 ){
			LogMsg("write data error!");
			return 1;
		}
		writeBuffer += writeBufferCount;
	}
	m_writeBufferPos = 0;
	return 0;
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
	if( iRet != 0 )
		return iRet;
		
	resultBody = body;
	return 0;
}
int32_t FastCgiSerialize::SerializeStdOut( FCGI_Header* header , void* body ){
	int32_t iRet;
	FCGI_StdOutBody* outBody = (FCGI_StdOutBody*)body;
	
	header->contentLength = outBody->bufferSize;
	iRet = SerializeHeaderBegin( header );
	if( iRet != 0 )
		return iRet;
	
	iRet = Write( outBody->buffer , outBody->bufferSize );
	if( iRet != 0 )
		return iRet;
	
	iRet = SerializeHeaderEnd( header );
	if( iRet != 0 )
		return iRet;
	
	return 0;
}
int32_t FastCgiSerialize::SerializeStdErr( FCGI_Header* header , void* body ){
	return SerializeStdOut(header,body);
}
int32_t FastCgiSerialize::SerializeData( FCGI_Header* header , void* body ){
	return SerializeStdOut(header,body);
}
int32_t FastCgiSerialize::SerializeEndRequest( FCGI_Header* header , void* body ){
	int32_t iRet;
	FCGI_EndRequestBody* requestBody = (FCGI_EndRequestBody*)body;
	
	header->contentLength = 8;
	iRet = SerializeHeaderBegin( header );
	if( iRet != 0 )
		return iRet;
	
	uint8_t buffer[8];
	buffer[0] = ( requestBody->appStatus >> 24 )& 0xFF;
	buffer[1] = ( requestBody->appStatus >> 16 )& 0xFF;
	buffer[2] = ( requestBody->appStatus >> 8 )& 0xFF;
	buffer[3] = ( requestBody->appStatus)& 0xFF;
	buffer[4] = requestBody->protocolStatus;
	buffer[5] = requestBody->reserved[0];
	buffer[6] = requestBody->reserved[1];
	buffer[7] = requestBody->reserved[2];
	iRet = Write( buffer , sizeof(buffer) );
	if( iRet != 0 )
		return iRet;
	
	iRet = SerializeHeaderEnd( header );
	if( iRet != 0 )
		return iRet;
	
	return 0;
}
int32_t FastCgiSerialize::SerializeHeaderBegin( FCGI_Header* header ){
	int32_t iRet;
	uint8_t buffer[8];
	header->paddingLength = (8- header->contentLength%8);
	if( header->paddingLength  == 8)
		header->paddingLength = 0;
		
	buffer[0] = header->version;
	buffer[1] = header->type;
	buffer[2] = ( header->requestId >> 8 )& 0xFF;
	buffer[3] = ( header->requestId)& 0xFF;
	buffer[4] = ( header->contentLength >> 8 )& 0xFF;
	buffer[5] = ( header->contentLength )& 0xFF;
	buffer[6] = header->paddingLength;
	buffer[7] = header->reserved;
	iRet = Write( buffer , sizeof(buffer) );
	if( iRet != 0 )
		return iRet;
	return 0;
}
int32_t FastCgiSerialize::SerializeHeaderEnd( FCGI_Header* header ){
	int32_t iRet;
	uint8_t buffer[8] = {0};
	iRet = Write( buffer , header->paddingLength );
	if( iRet != 0 )
		return iRet;
	return 0;
}
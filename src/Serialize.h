#ifndef __FAST_CGI_SERIALIZE_H__
#define __FAST_CGI_SERIALIZE_H__

#include "FastCgiRequest.h"
#include "FastCgiResponse.h"
#include <stdint.h>
#include <string>

namespace fish{

#define FCGI_MAX_READ_BUFFER_SIZE 8192
#define FCGI_MAX_WRITE_BUFFER_SIZE 8192

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)
#define FCGI_RESPONDER 1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER 3

typedef struct {
	uint8_t version;
    uint8_t type;
    uint8_t requestIdB1;
	uint8_t requestIdB0;
    uint8_t contentLengthB1;
	uint8_t contentLengthB0;
	uint8_t paddingLength;
	uint8_t reserved;
	uint8_t* content;
} FCGI_Header;

typedef struct {
	uint16_t role;
	uint8_t flags;
	uint8_t reserved[5];
} FCGI_BeginRequestBody;

typedef struct {
	uint32_t appStatus;
	uint8_t protocolStatus;
	uint8_t reserved[3];
} FCGI_EndRequestBody;

typedef struct{
	uint8_t* buffer;
	uint16_t bufferSize; 
} FCGI_StdInBody,FCGI_DataBody,FCGI_StdOutBody,FCGI_StdErrBody;

typedef struct {
	uint32_t nameLength; 
	uint32_t valueLength;
	uint8_t* nameData;
	uint8_t* valueData;
} FCGI_NameValuePair;

typedef struct FCGI_ParamsBody{
	struct FCGI_ParamsBody* next;
	FCGI_NameValuePair param;
}FCGI_ParamsBody;

typedef struct FCGI_GetValuesBody{
	struct FCGI_GetValuesBody* next;
	FCGI_NameValuePair param;
}FCGI_GetValuesBody;

typedef struct {
	uint8_t type;    
	uint8_t reserved[7];
} FCGI_UnknownTypeBody;

class FastCgiSerialize{

public:
	FastCgiSerialize( int socket );
	~FastCgiSerialize();

public:
	int32_t WriteResponseItem( FCGI_Header* header , void* body );
	int32_t FinishResponseItem();
	int32_t ReadRequestItem( FCGI_Header*& header , void*& body  );
	int32_t FreeRequestItem( FCGI_Header*& header , void*& body );
	
private:
	int32_t Write( uint8_t* buffer , int size );
	int32_t WriteFlush();
	int32_t Read( uint8_t* buffer , int size );
	int32_t ReadContent( FCGI_Header* header , uint8_t* buffer , int size );
	int32_t ReadPadding( FCGI_Header* header , uint8_t* buffer , int size );
	void LogMsg( const std::string& strMsg );
	
private:
	int32_t SerializeStdOut( FCGI_Header* header , void* body );
	int32_t SerializeStdErr( FCGI_Header* header , void* body );
	int32_t SerializeData( FCGI_Header* header , void* body );
	int32_t SerializeEndRequest( FCGI_Header* header , void* body );
	int32_t SerializeHeaderBegin( FCGI_Header* header );
	int32_t SerializeHeaderEnd( FCGI_Header* header );
	
	int32_t DeSerializeHeader( FCGI_Header*& header );
	int32_t DeSerializeBeginRequestBody( FCGI_Header* header , void*& resultBody );
	int32_t DeSerializeParamsBody( FCGI_Header* header , void*& resultBody );
	int32_t DeSerializeStdinBody( FCGI_Header* header , void*& resultBody );
	int32_t DeSerializeDataBody( FCGI_Header* header , void*& resultBody );
	int32_t DeSerializeGetValuesBody( FCGI_Header* header , void*& resultBody );
	int32_t DeSerializeUnknownBody( FCGI_Header* header , void*& resultBody );
	
private:
	unsigned char m_readBuffer[FCGI_MAX_READ_BUFFER_SIZE];
	uint32_t m_readBufferPos;
	uint32_t m_readBufferSize;
	unsigned char m_writeBuffer[FCGI_MAX_WRITE_BUFFER_SIZE];
	uint32_t m_writeBufferPos;
	uint32_t m_writeBufferSize;
	int m_socket;
};

}

#endif
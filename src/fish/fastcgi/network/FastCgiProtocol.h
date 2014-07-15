#ifndef __FAST_CGI_PROTOCOL_H__
#define __FAST_CGI_PROTOCOL_H__

#include <list>

namespace fish{
namespace fastcgi{
namespace network{

//fast cgi type
enum{
	FCGI_BEGIN_REQUEST = 1,
	FCGI_ABORT_REQUEST = 2,
	FCGI_END_REQUEST = 3,
	FCGI_PARAMS = 4,
	FCGI_STDIN = 5,
	FCGI_STDOUT = 6,
	FCGI_STDERR = 7,
	FCGI_DATA = 8,
	FCGI_GET_VALUES = 9,
	FCGI_GET_VALUES_RESULT = 10,
	FCGI_UNKNOWN_TYPE = 11,
	FCGI_MAXTYPE = 11,
};

enum{
	FCGI_RESPONDER = 1,
	FCGI_AUTHORIZER = 2,
	FCGI_FILTER = 3,
};

typedef struct {
	uint8_t version;
    uint8_t type;
    uint16_t requestId;
    uint16_t contentLength;
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

class FastCgiProtocol{
public:
	FastCgiProtocol();
	~FastCgiProtocol();
	
public:
	int32_t DeSerializeRequest( const std::list<FCGI_Header*>& headerList, FastCgiRequest& request );
	int32_t SerializeResponse( const FastCgiResponse& response , std::string& strResponse );
	int32_t DeSerializeGetValues( const FCGI_Header* header , FastCgiRequest& request );
	int32_t SerializeGetValuesResult( const std::map<std::string,std::string>& response , std::string& strResponse );
	
private:
	int32_t DeSerializeParams( const FCGI_Header* headerList, FastCgiRequest& request );
	int32_t DeSerializeGetValues( const FCGI_Header* headerList, FastCgiRequest& request );
	int32_t DeSerializeStdIn( const FCGI_Header* headerList, FastCgiRequest& request );
	int32_t DeSerializeData( const FCGI_Header* headerList, FastCgiRequest& request );
	int32_t DeSerializeBeginRequest( const FCGI_Header* headerList, FastCgiRequest& request );
	
	int32_t SerializeEndRequest( const FastCgiResponse& response , std::string& strResponse );
	int32_t SerializeStdOut( const FastCgiResponse& response , std::string& strResponse );
	int32_t SerializeStdErr( const FastCgiResponse& response , std::string& strResponse );
	int32_t SerializeData( const FastCgiResponse& response , std::string& strResponse );
	
	int32_t SerializeHeader( const FastCgiResponse& response , std::string& strResponse );
	int32_t SerializePadding( const FastCgiResponse& response , std::string& strResponse );

};

}
}
}

#endif

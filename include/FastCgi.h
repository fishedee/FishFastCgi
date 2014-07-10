#ifndef __FISH_FAST_CGI_H__
#define __FISH_FAST_CGI_H__

#include "FastCgiRequest.h"
#include "FastCgiResponse.h"
#include <stdint.h>
#include <tr1/functional>
#include <string>

namespace fish{

class FastCgi{

public:
	typedef std::tr1::function<void( const FastCgiRequest& request , FastCgiResponse& response )> CallBackFun;

public:
	FastCgi();
	~FastCgi();
	
public:
	int32_t SetNetwork( const std::string& strIP , uint16_t dwPort );
	int32_t SetProcess( uint32_t dwProcess );
	int32_t SetThreadPerProcess( uint32_t dwThread );
	int32_t SetCallBack( const CallBackFun& callback );
	int32_t Run();
	std::string GetLastMsg();

public:
	int32_t HandleRequest();
	
private:
	void LogMsg( const std::string& strMsg );
	void SetLastMsg( const std::string& m_strLastMsg );
	int32_t BindSocket();
	int32_t DeSerializeRequest(  int clientSocket , FastCgiRequest& request );
	int32_t SerializeResponse( int clientSocket , const FastCgiResponse& response );
	
private:
	std::string m_strLastMsg;
	std::string m_strIP;
	uint16_t m_dwPort;
	uint32_t m_dwProcess;
	uint32_t m_dwThread;
	int m_serverSocket;
	CallBackFun m_callback;

};

}

#endif

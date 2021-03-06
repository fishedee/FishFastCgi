#ifndef __FAST_CGI_NETWORK_H__
#define __FAST_CGI_NETWORK_H__

#include <tr1/functional>
#include <tr1/memory>
#include <vector>
#include <stdint.h>
#include "ServerSocket.h"
#include "ClientSocket.h"
#include "FastCgiSocket.h"
#include "FastCgiRequest.h"
#include "FastCgiResponse.h"

namespace fish{
namespace fastcgi{
namespace network{

class NetworkListener{
public:
	virtual void OnRequest( const FastCgiRequest&request , FastCgiResponse& response ) = 0;
};

class Network:FastCgiSocketListener{

public:
	Network();
	~Network();
	void ListenPort( uint16_t dwPort );
	void ListenUnixAddress( const std::string& strAddress );
	void SetClientThread( uint32_t dwThread );
	void SetListener( NetworkListener& listener );
	int32_t Run();
	int32_t Wait();
	
public:
	void OnRequest( const FastCgiRequest&request , FastCgiResponse& response );
	
private:
	static void* ThreadHelper( void* );
	
public:
	uint32_t m_dwThread;
	NetworkListener* m_listener;
	std::vector<FastCgiSocket*> m_vecClientSockets;
	std::vector<pthread_t> m_vecThread;
	ServerSocket m_serverSocket;
	
};

}
}
}

#endif

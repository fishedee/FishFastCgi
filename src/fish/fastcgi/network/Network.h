#ifndef __FAST_CGI_NETWORK_H__
#define __FAST_CGI_NETWORK_H__

#include <tr1/functional>
#include <tr1/memory>
#include "ServerSocket.h"
#include "ClientSocket.h"
#include ""

namespace fish{
namespace fastcgi{
namespace network{

class Network{

public:
	typedef std::tr1::shared_ptr<void(const FastCgiRequest&request,FastCgiResponse& response)> RequestHandler;

public:
	Network();
	~Network();
	void ListenPort( uint16_t dwPort );
	void SetClientThread( uint32_t dwThread );
	void SetRequestHandler( const RequestHandler& requestHandler );
	int32_t Run();

public:
	uint32_t m_dwThread;
	RequestHandler m_requestHandler;
	vector<FastCgiSocket> m_vecClientSockets;
	ServerSocket m_serverSocket;
	
};

}
}
}

#endif

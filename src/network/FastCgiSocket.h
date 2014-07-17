#ifndef __FAST_CGI_SOCKET_H__
#define __FAST_CGI_SOCKET_H__

#include <map>
#include <list>
#include "FastCgiRequest.h"
#include "FastCgiResponse.h"
#include "ClientSocket.h"
#include "FastCgiProtocol.h"

namespace fish{
namespace fastcgi{
namespace network{

class FastCgiSocketListener{
public:
	virtual void OnRequest( const FastCgiRequest&request , FastCgiResponse& response ) = 0;
};

class FastCgiSocketData:public ClientSocketData{
public:
	FastCgiRequest request;
	FastCgiResponse response;
};

class FastCgiSocket:public ClientSocketListener{
public:
	FastCgiSocket( ClientSocket& clientSocket );
	virtual ~FastCgiSocket();

public:
	void SetRequestListener( FastCgiSocketListener& listener );
	int32_t Run();
	ClientSocket* GetClientSocket();

public:
	ClientSocketData* OnConnected();
	void OnRead( ClientSocketData* data );
	void OnClose( ClientSocketData* data  );
	
	void ReadSingleRequest( const char* buffer , FastCgiSocketData* data );
	
private:
	FastCgiSocketListener* m_listener;
	ClientSocket& m_clientSocket;
	FastCgiProtocol m_protocol;
	
};

}
}
}

#endif

#ifndef __FAST_CGI_SOCKET_H__
#define __FAST_CGI_SOCKET_H__

#include <map>
#include <list>
#include "ClientSocket.h"
#include "FastCgiProtocol.h"

namespace fish{
namespace fastcgi{
namespace network{

class FastCgiSocketListener{
public:
	virtual void OnRequest( const FastCgiRequest&request , FastCgiResponse& response );
};

class FastCgiSocket:public ClientSocketListener{
public:
	FastCgiSocket( ClientSocket& clientSocket );
	~FastCgiSocket();

public:
	void SetRequestListener( FastCgiSocketListener& listener );
	int32_t Run();

public:
	void OnConnected( int socket );
	void OnRead( int socket , const std::string& request , std::string& response );
	void OnClose( int socket );
	
	void OnReadHeader( int socket , const char* buffer , std::string& response );
	void OnReadStdInFinish( int socket , uint16_t requestId );
	void OnReadGetValues( int socket , FCGI_Header* header );
	void OnCloseRequest( int socket , uint16_t requestId );
	
private:
	std::map<int,std::map<uint16_t,std::list<FCGI_Header*> > > m_mapHeaderList;
	FastCgiSocketListener* m_listener;
	FastCgiProtocol m_protocol;
	ClientSocket m_clientSocket;
};

}
}
}

#endif

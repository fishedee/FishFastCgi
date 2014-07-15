#ifndef __FAST_CGI_SERVER_SOCKET_H__
#define __FAST_CGI_SERVER_SOCKET_H__

#include "stdint.h"

namespace fish{
namespace fastcgi{
namespace network{

class ServerSocket{
public:
	ServerSocket();
	~ServerSocket();
	
public:
	int32_t ListenPort( uint16_t dwPort );
	int GetSocket();
	
private:
	int m_serverSocket;
	
};

}
}
}
#endif

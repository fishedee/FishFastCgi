#ifndef __FAST_CGI_SERVER_SOCKET_H__
#define __FAST_CGI_SERVER_SOCKET_H__

#include "stdint.h"
#include <string>

namespace fish{
namespace fastcgi{
namespace network{

class ServerSocket{
public:
	ServerSocket();
	~ServerSocket();
	
public:
	int32_t ListenPort( uint16_t dwPort );
	int32_t ListenUnixSokcet( const std::string& strAddress );
	int GetSocket();
	bool IsUnixSocket();
	
private:
	int m_serverSocket;
	bool m_isUnixSocket;
	
};

}
}
}
#endif

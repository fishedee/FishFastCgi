#ifndef __FISH_FAST_CGI_H__
#define __FISH_FAST_CGI_H__

#include "FastCgiRequest.h"
#include "FastCgiResponse.h"
#include "Network.h"
#include <stdint.h>
#include <tr1/functional>
#include <string>

namespace fish{
namespace fastcgi{

class FastCgi:public fish::fastcgi::network::NetworkListener{

public:
	typedef std::tr1::function<void( const FastCgiRequest& request , FastCgiResponse& response )> CallBackFun;

public:
	FastCgi();
	~FastCgi();
	
public:
	void SetNetworkPort( uint16_t dwPort );
	void SetNetworkThread( uint32_t dwNetworkThread );
	void SetProcess( uint32_t dwProcess );
	void SetCallBack( const CallBackFun& callback );
	int32_t Run();
	
public:
	void OnRequest( const FastCgiRequest&request , FastCgiResponse& response );
	
private:
	uint16_t m_dwPort;
	uint32_t m_dwProcess;
	uint32_t m_dwNetworkThread;
	FastCgi::CallBackFun m_callback;

};

}
}

#endif

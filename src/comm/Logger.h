#ifndef __FAST_CGI_LOGGER_H__
#define __FAST_CGI_LOGGER_H__

#include <string>

namespace fish{
namespace fastcgi{
namespace comm{

class Logger{
public:
	static int32_t Init();
	static void Err( const std::string& strLog );
	static void Info( const std::string& strLog );
	static void Debug( const std::string& strLog ){
		//fprintf(stdout,"%s\n",strLog.c_str());
	}
};

}
}
}

#endif

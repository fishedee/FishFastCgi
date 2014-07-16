#include "Logger.h"
#include <stdio.h>

namespace fish{
namespace fastcgi{
namespace comm{

int32_t Logger::Init(){
	return 0;
}

void Logger::Err( const std::string& strLog ){
	fprintf(stderr,"%s\n",strLog.c_str());
}

void Logger::Info( const std::string& strLog ){
	fprintf(stdout,"%s\n",strLog.c_str());
}
/*
void Logger::Debug( const std::string& strLog ){
	fprintf(stdout,"%s\n",strLog.c_str());
}
*/

}
}
}
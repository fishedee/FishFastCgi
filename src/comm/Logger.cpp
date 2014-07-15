#include "Logger.h"
#include <stdio.h>

namespace fish{
namespace fastcgi{
namespace comm{

int32_t Logger::Init(){
	return 0;
}

void Logger::Err( const std::string& strLog ){
	fprintf(stderr,(strLog + "\n").c_str());
}

void Logger::Info( const std::string& strLog ){
	fprintf(stdout,(strLog + "\n").c_str());
}

void Logger::Debug( const std::string& strLog ){
	fprintf(stdout,(strLog + "\n").c_str());
}

}
}
}
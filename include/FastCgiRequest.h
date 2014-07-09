#ifndef __FISH_FAST_CGI_REQUEST_H__
#define __FISH_FAST_CGI_REQUEST_H__

#include <stdint.h>
#include <string>
#include <map>

namespace fish{

class FastCgiRequest{

public:
	FastCgiRequest(){
	}
	
	~FastCgiRequest(){
	}
	
public:
	//Http请求头部
	void SetHeader( const std::map<std::string,std::string>& mapHeader ){
		m_mapHeader = mapHeader;
	}
	
	const std::map<std::string,std::string>& GetHeader(){
		return m_mapHeader;
	}
	
	//Http请求参数
	void SetParam( const std::map<std::string,std::string>& mapParam ){
		m_mapParam = mapParam;
	}
	
	const std::map<std::string,std::string>& GetParam(){
		return m_mapParam;
	}
	
private:
	std::map<std::string,std::string> m_mapHeader;
	std::map<std::string,std::string> m_mapParam;

};

}

#endif
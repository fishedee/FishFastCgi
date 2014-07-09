#ifndef __FISH_FAST_CGI_RESPONSE_H__
#define __FISH_FAST_CGI_RESPONSE_H__

#include <stdint.h>
#include <string>
#include <map>

namespace fish{

class FastCgiResponse{

public:
	FastCgiResponse(){
	}
	
	~FastCgiResponse(){
	}
	
public:
	//Http回复头部
	void SetHeader( const std::map<std::string,std::string>& mapHeader ){
		m_mapHeader = mapHeader;
	}
	
	const std::map<std::string,std::string>& GetHeader(){
		return m_mapHeader;
	}
	
	//Http回复码
	void SetRetCode( uint32_t dwRetCode ){
		m_dwRetCode = dwRetCode;
	}
	
	uint32_t GetRetCode(){
		return m_dwRetCode;
	}
	
	
	//Http回复Body
	void SetBody( const std::string& strBody ){
		m_strBody = strBody;
	}
	
	std::string GetBody(){
		return m_strBody;
	}
	
private:
	std::map<std::string,std::string> m_mapHeader;
	uint32_t m_dwRetCode;
	std::string m_strBody;

};

}

#endif
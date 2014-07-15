#ifndef __FISH_FAST_CGI_RESPONSE_H__
#define __FISH_FAST_CGI_RESPONSE_H__

#include <stdint.h>
#include <string>
#include <map>

namespace fish{

class FastCgiResponse{

public:
	FastCgiResponse(){
		m_protocolStatus = 200;
		m_appStatus = 0;
	}
	
	~FastCgiResponse(){
	}
	
public:
	//Http请求id
	void SetRequestId( uint16_t requestId ){
		m_requestId = requestId;
	}
	
	uint16_t GetRequestId()const{
		return m_requestId;
	}
	
	uint16_t& GetRequestId(){
		return m_requestId;
	}
	
	//Http协议状态码
	void SetProtocolStatus( uint8_t protocolStatus ){
		m_protocolStatus = protocolStatus;
	}
	
	uint8_t GetProtocolStatus()const{
		return m_protocolStatus;
	}
	
	uint8_t& GetProtocolStatus(){
		return m_protocolStatus;
	}
	
	//Http状态吗
	void SetAppStatus( uint32_t appStatus ){
		m_appStatus = appStatus;
	}
	
	uint32_t GetAppStatus()const{
		return m_appStatus;
	}
	
	uint32_t& GetAppStatus(){
		return m_appStatus;
	}
	
	//Http标准Data
	void SetData( const std::string& strData ){
		m_strData = strData;
	}
	
	const std::string& GetData()const{
		return m_strData;
	}
	
	std::string& GetData(){
		return m_strData;
	}
	
	//Http标准输出
	void SetOut( const std::string& strOut ){
		m_strOut = strOut;
	}
	
	const std::string& GetOut()const{
		return m_strOut;
	}
	
	std::string& GetOut(){
		return m_strOut;
	}
	
	//Http标准错误
	void SetErr( const std::string& strErr ){
		m_strErr = strErr;
	}
	
	const std::string& GetErr()const{
		return m_strErr;
	}
	
	std::string& GetErr(){
		return m_strErr;
	}
	
private:
	std::string m_strErr;
	std::string m_strOut;
	std::string m_strData;
	uint32_t m_appStatus;
	uint8_t m_protocolStatus;
	uint16_t m_requestId;
};

}

#endif
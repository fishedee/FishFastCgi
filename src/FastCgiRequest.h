#ifndef __FISH_FAST_CGI_REQUEST_H__
#define __FISH_FAST_CGI_REQUEST_H__

#include <stdint.h>
#include <string>
#include <map>

namespace fish{
namespace fastcgi{

class FastCgiRequest{

public:
	FastCgiRequest(){
		Clear();
	}
	
	~FastCgiRequest(){
	}
	
public:
	//请求flag
	void SetFlag( uint8_t flag ){
		m_flag = flag;
	}
	
	uint8_t GetFlag()const{
		return m_flag;
	}
	
	uint8_t& GetFlag(){
		return m_flag;
	}
	
	//请求requestId
	void SetRequestId( uint16_t requestId ){
		m_requestId = requestId;
	}
	
	uint16_t GetRequestId()const{
		return m_requestId;
	}
	
	uint16_t& GetRequestId(){
		return m_requestId;
	}
	
	//请求role
	void SetRole( uint16_t role ){
		m_role = role;
	}
	
	uint16_t GetRole()const{
		return m_role;
	}
	
	uint16_t& GetRole(){
		return m_role;
	}
	
	//请求参数
	void SetParams( const std::map<std::string,std::string>& mapParams ){
		m_mapParams = mapParams;
	}
	
	const std::map<std::string,std::string>& GetParams()const{
		return m_mapParams;
	}
	
	std::map<std::string,std::string>& GetParams(){
		return m_mapParams;
	}
	
	//请求stdin
	void SetIn( const std::string& strStdIn ){
		m_strStdIn = strStdIn;
	}
	
	const std::string& GetIn()const{
		return m_strStdIn;
	}
	
	std::string& GetIn(){
		return m_strStdIn;
	}
	
	//请求data
	void SetData( const std::string& strData ){
		m_strData = strData;
	}
	
	const std::string& GetData()const{
		return m_strData;
	}
	
	std::string& GetData(){
		return m_strData;
	}
	
public:
	void Clear(){
		m_mapParams.clear();
		m_strData.clear();
		m_strStdIn.clear();
	}
	
private:
	std::map<std::string,std::string> m_mapParams;
	std::string m_strStdIn;
	std::string m_strData;
	uint16_t m_role;
	uint16_t m_requestId;
	uint8_t m_flag;
};

}
}

#endif
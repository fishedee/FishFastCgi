#include "FastCgi.h"
#include <iostream>
#include <stdio.h>
#include <string>
using namespace std;
using namespace fish;

#define TO_STRING(a) to_string(a).c_str()
void go( const FastCgiRequest& request , FastCgiResponse& response ){
	printf("request in [flag:%s][requestId:%s][role:%s][in:%s]",
		TO_STRING(request.GetFlag()),
		TO_STRING(request.GetRequestId()),
		TO_STRING(request.GetRole()),
		request.GetIn().c_str());
	for( map<string,string>::const_iterator it = request.GetParams().begin();
		it != request.GetParams().end() ; ++it )
		printf("[params:[key:%s][value:%s]]",
			it->first.c_str(),
			it->second.c_str());
	printf("\n");
	
	response.SetOut("Fish World!");
	response.SetProtocolStatus(200);
	response.SetAppStatus(0);
}

int main(){
	FastCgi cgi;
	int32_t iRet;
	iRet = cgi.SetNetwork("127.0.0.1",4123);
	if( iRet != 0 ){
		printf("%s\n",cgi.GetLastMsg().c_str());
		return iRet;
	}
	
	iRet = cgi.SetProcess(1);
	if( iRet != 0 ){
		printf("%s\n",cgi.GetLastMsg().c_str());
		return iRet;
	}
	
	iRet = cgi.SetThreadPerProcess(1);
	if( iRet != 0 ){
		printf("%s\n",cgi.GetLastMsg().c_str());
		return iRet;
	}
	
	iRet = cgi.SetCallBack(go);
	if( iRet != 0 ){
		printf("%s\n",cgi.GetLastMsg().c_str());
		return iRet;
	}
	
	iRet = cgi.Run();
	if( iRet != 0 ){
		printf("%s\n",cgi.GetLastMsg().c_str());
		return iRet;
	}
	
	return 0;
}

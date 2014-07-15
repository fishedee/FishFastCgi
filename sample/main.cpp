#include "FastCgi.h"
#include <iostream>
#include <stdio.h>
#include <string>
using namespace std;
using namespace fish;

#define TO_STRING(a) to_string(a).c_str()
void go( const FastCgiRequest& request , FastCgiResponse& response ){
	/*
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
	*/
	
	response.SetOut(
		"Content-type: text/html \r\n\r\n"
		"<html><head></head><body>Hello World!</body><html>\n"
	);
	response.SetProtocolStatus(200);
	response.SetAppStatus(0);
}

int main(){
	FastCgi cgi;
	int32_t iRet;
	
	cgi.SetNetworkPort(4123);
	cgi.SetNetworkThread(10);
	cgi.SetProcess(1);
	cgi.SetCallBack(go);

	iRet = cgi.Run();
	if( iRet != 0 ){
		printf("%s\n",cgi.GetLastMsg().c_str());
		return iRet;
	}
	
	return 0;
}

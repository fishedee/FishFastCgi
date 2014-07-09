#include "FastCgi.h"
#include <iostream>
#include <stdio.h>
using namespace std;
using namespace fish;

void go( const FastCgiRequest& request , FastCgiResponse& response ){
	response.SetBody("Hello World!");
	printf("Hello World!\n");
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

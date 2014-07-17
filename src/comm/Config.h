#ifndef __FAST_CGI_CONFIG_H__
#define __FAST_CGI_CONFIG_H__

namespace fish{
namespace fastcgi{
namespace comm{

class Config{
public:
	static bool GetIsRun( ){
		return ms_isRun;
	}
	static void SetIsRun( bool isRun ){
		ms_isRun = isRun; 
	}

private:
	static volatile bool ms_isRun;
	
};

}
}
}
#endif

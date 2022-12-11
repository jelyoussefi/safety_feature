#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "safety_feature.hpp"

using namespace std;

static void callback(void* userParam) 
{
	std::cout<<"\t -> User Callback : "<<(int)userParam << " State "<<TimerState()<< std::endl;
}

static bool wait_key(uint32_t timeout, int *c) 
{
    struct termios oldSettings, newSettings;
    tcgetattr( fileno( stdin ), &oldSettings );
    newSettings = oldSettings;
    newSettings.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr( fileno( stdin ), TCSANOW, &newSettings );    

    fd_set set;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = timeout*1000;

    FD_ZERO( &set );
    FD_SET( fileno( stdin ), &set );

    int res = select( fileno( stdin )+1, &set, NULL, NULL, &tv );

 	if( res > 0 ) {
        read( fileno( stdin ), c, 1 );
    }

    tcsetattr( fileno( stdin ), TCSANOW, &oldSettings );

    return (res > 0);
}

     
int main(int argc, char* argv[])
{
	Status_t ret;
	State_t state;
	uint32_t timeout = 200;
	Image_t custImage = {0,0,RGBA_8888, NULL};

	auto env_p = getenv("SF_TIMEOUT");
	if ( env_p ) {
		timeout = stoi(env_p);
	}

	ret = TimerStart(timeout, callback, (void*)1234, custImage);
	if ( ret != OK ) {
		std::cout<<"Timer_Start failed"<<std::endl;
		return -1;
	}

	while ( true ) {
		int c;
		if (!wait_key(timeout-10, &c)) {
			TimerReset();
		}
		else {
			std::cout<<"\nKey Pressed: "<<c<<std::endl;

			if ( c == 'c' ) {
				break;
			}
		}
	}

	TimerStop();
	return 0;
}

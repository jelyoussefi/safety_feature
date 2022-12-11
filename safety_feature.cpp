#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <chrono>
#include <linux/hpet.h>
#include <linux/watchdog.h>


#include "safety_feature.hpp"

using namespace std::chrono;
typedef high_resolution_clock::time_point Time_t;

inline Time_t TimeNow() {
	return high_resolution_clock::now();
}

inline float TimeElapsed(Time_t t_start) {
	high_resolution_clock::time_point t_stop = TimeNow();
	duration<double, std::milli> diff = duration_cast<duration<double>>(t_stop - t_start);
	return diff.count();
}

//-------------------------------------------------------------------------------------------------------
//              Local data
//-------------------------------------------------------------------------------------------------------

static uint32_t timeout = 0;
static Callback_t callback = NULL;
static void* userParam = NULL;
static Image_t customImage;
static State_t state = IDLE;
static Time_t startTime;
static sig_t oldSignal;
static int hpet_fd ;
static int watchdog_fd ;


//-------------------------------------------------------------------------------------------------------
//              Loal Functions
//-------------------------------------------------------------------------------------------------------

static void hpetCallback(int val)
{
    std::cout << "\thpet callback : " << int(TimeElapsed(startTime)) << " ms" << std::endl;
    state = BLACKENING;
    if ( callback ) {
	callback(userParam);
    }
}

//-------------------------------------------------------------------------------------------------------
// 				Public Functions
//-------------------------------------------------------------------------------------------------------

Status_t TimerStart(uint32_t TimeoutMs, Callback_t Callback, void* UserParam, Image_t& CustomImage ) 
{
    int value;

    timeout = 1000/TimeoutMs;
    callback = Callback;
    userParam = UserParam;
    customImage = CustomImage;

 
    if ( (oldSignal = signal(SIGIO, hpetCallback)) == SIG_ERR) {
    	std::cout<<"failed to set signal handler"<<std::endl;
        return ERROR;
    }

    hpet_fd = open("/dev/hpet", O_RDONLY);
    if (hpet_fd < 0) {
        std::cout<<"Failed to open /dev/hpet"<<std::endl;
        return ERROR;
    }

    if ( (fcntl(hpet_fd, F_SETOWN, getpid()) == 1) || 
         ((value = fcntl(hpet_fd, F_GETFL)) == 1) ||
         (fcntl(hpet_fd, F_SETFL, value | O_ASYNC) == 1)) {
         std::cout<<"fcntl failed"<<std::endl;
        return ERROR;
    }

    watchdog_fd = open("/dev/watchdog", O_RDWR);
    if (watchdog_fd < 0) {
        std::cout<<"Failed to open /dev/watchdog"<<std::endl;
        return ERROR;
    }
	

    state = IDLE;
    return TimerReset();
}

Status_t TimerReset()
{
    if ( state == IDLE ) {
	state = MONITORING;
	int wd_timeout = 1;
    	int ret = ioctl(watchdog_fd, WDIOC_SETTIMEOUT, &wd_timeout);
	if ( ret < 0 ) {	
  		std::cout<<"WDIOC_SETTIMEOUT failed"<<std::endl;
        	return ERROR;
	}	
    }

    if (ioctl(hpet_fd, HPET_IE_OFF, 0) < 0) {
        std::cout<<"HPET_IE_OFF failed"<<std::endl;
        return ERROR;
    }

    if (ioctl(hpet_fd, HPET_IRQFREQ, timeout) < 0) {
        std::cout<<"HPET_IRQFREQ failed"<<std::endl;
        return ERROR;
    }

   if (ioctl(hpet_fd, HPET_IE_ON, 0) < 0) {
        std::cout<<"HPET_IE_ON failed"<<std::endl;
        return ERROR;
    }

    if (ioctl(watchdog_fd, WDIOC_KEEPALIVE, 0) < 0) {
	std::cout<<"WDIOC_KEEPALIVE failed"<<std::endl;
        return ERROR;
    }

    startTime = TimeNow();

    return OK;
}

Status_t TimerStop()
{
    if (ioctl(hpet_fd, HPET_IE_OFF, 0) < 0) {
        std::cout<<"HPET_IE_OFF failed"<<std::endl;
        return ERROR;
    }
    close(hpet_fd);
    close(watchdog_fd);

    signal(SIGIO, oldSignal);
    return OK;
}

State_t  TimerState()
{
    return state;
}

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
static int fd ;

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

    fd = open("/dev/hpet", O_RDONLY);
    if (fd < 0) {
        std::cout<<"Failed to open /dev/hpet"<<std::endl;
        return ERROR;
    }

    if ( (fcntl(fd, F_SETOWN, getpid()) == 1) || 
         ((value = fcntl(fd, F_GETFL)) == 1) ||
         (fcntl(fd, F_SETFL, value | O_ASYNC) == 1)) {
         std::cout<<"fcntl failed"<<std::endl;
        return ERROR;
    }
  
    state = MONITORING;

    return TimerReset();
}

Status_t TimerReset()
{
    if ( state == IDLE ) {
        return ERROR;
    }

    if (ioctl(fd, HPET_IE_OFF, 0) < 0) {
        std::cout<<"HPET_IE_OFF failed"<<std::endl;
        return ERROR;
    }

    if (ioctl(fd, HPET_IRQFREQ, timeout) < 0) {
        std::cout<<"HPET_IRQFREQ failed"<<std::endl;
        return ERROR;
    }

   if (ioctl(fd, HPET_IE_ON, 0) < 0) {
        std::cout<<"HPET_IE_ON failed"<<std::endl;
        return ERROR;
    }

    startTime = TimeNow();

    return OK;
}

Status_t TimerStop()
{
    if (ioctl(fd, HPET_IE_OFF, 0) < 0) {
        std::cout<<"HPET_IE_OFF failed"<<std::endl;
        return ERROR;
    }

    signal(SIGIO, oldSignal);
    return OK;
}

State_t  TimerState()
{
    return state;
}

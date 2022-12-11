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
#include <linux/hpet.h>


#include "safety_feature.hpp"

//-------------------------------------------------------------------------------------------------------
//              Local data
//-------------------------------------------------------------------------------------------------------
static uint32_t timeout = 0;
static Callback_t callback = NULL;
static void* userParam = NULL;
static Image_t customImage;
static State_t state = IDLE;

static int fd ;

//-------------------------------------------------------------------------------------------------------
//              Loal Functions
//-------------------------------------------------------------------------------------------------------

static void hpet_sigio(int val)
{
    fprintf(stderr, "hpet_sigio: called\n");
}

//-------------------------------------------------------------------------------------------------------
// 				Public Functions
//-------------------------------------------------------------------------------------------------------

Status_t Timer_Start(uint32_t TimeoutMs, Callback_t Callback, void* UserParam, Image_t& CustomImage ) 
{
    int value;

    timeout = TimeoutMs;
    callback = Callback;
    userParam = UserParam;
    customImage = CustomImage;

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

    if (ioctl(fd, HPET_IRQFREQ, TimeoutMs) < 0) {
        std::cout<<"HPET_IRQFREQ failed"<<std::endl;
        return ERROR;
    }

    if (ioctl(fd, HPET_IE_ON, 0) < 0) {
        std::cout<<"HPET_IE_ON failed"<<std::endl;
        return ERROR;
    }

    state = MONITORING;

    return OK;
}

Status_t Timer_Reset()
{
    if ( state != MONITORING ) {
        return ERROR;
    }
    return OK;
}

Status_t Timer_Stop()
{
    return OK;
}

State_t  Timer_State()
{
    return IDLE;
}

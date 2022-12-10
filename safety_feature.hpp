#pragma once

#include <stdint.h>

typedef enum {
    OK,
    ERROR
} Status_t;

typedef enum {
    RGB_565,
    RGB_888,
    RGBA_8888,
    GRAYSCALE_8,
    GRAYSCALE_16	
} ImageFormat_t;

typedef struct {
     uint32_t      width;
     uint32_t      height;
     ImageFormat_t format;
     void*         data;
} Image_t;

typedef enum {
    IDLE,
    MONITORING,
    BLACKENING
} State_t;

typedef void (*Callback_t)(void*);

//-------------------------------------------------------------------------------------------------------
// 				Public Functions
//-------------------------------------------------------------------------------------------------------

Status_t Timer_Start(uint32_t TimeoutMs, Callback_t Callback, void* UserParam, Image_t& CustomImage );
Status_t Timer_Reset();
Status_t Timer_Stop();
State_t  Timer_State();

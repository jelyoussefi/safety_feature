#include "safety_feature.hpp"

//-------------------------------------------------------------------------------------------------------
// 				Public Functions
//-------------------------------------------------------------------------------------------------------

Status_t Timer_Start(uint32_t TimeoutMs, Callback_t Callback, void* UserParam, Image_t& CustomImage ) 
{
    return OK;
}

Status_t Timer_Reset()
{
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

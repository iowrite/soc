#include <stdint.h>
#include <stdio.h>
#include "sox_private.h"
#include "sox.h"
int8_t soe_init()
{


}

static float s_lastCur = 0;
int8_t soe_task()
{
    if(*g_cur > 0){
        *g_accChgWH += *g_cur * *g_grpVol /3600;
    }else{
        *g_accDsgWH += *g_cur * *g_grpVol /3600;
    }
    static int state = 0, lastState = 0;
    switch(state){
        case 0:                                         // standby
            lastState = state;
            if(*g_cur > CUR_WINDOW_A)
            {

                state = 1;
            }else if(*g_cur < -CUR_WINDOW_A)
            {
                state = 3;
            }
            
            break;
        case 1:                                         // charge
            lastState = state;
            if(*g_cur> 0)
            {
                *g_sigChgWH += *g_cur * *g_grpVol /3600;
            }
            if(*g_cur < -CUR_WINDOW_A)
            {
                state = 2;
            }
            break;
        case 2:                                         // charge to discharge
            *g_accChgWH = 0;
            *g_accDsgWH = 0;
            lastState = state;
            state = 3;
            break;
        case 3:                                         // discharge        
            lastState = state;
            if(*g_cur < 0){
                *g_sigDsgWH += *g_cur * *g_grpVol /3600;
            }
            if(*g_cur > CUR_WINDOW_A)
            {
                state = 4;
            }
            break;
        case 4:                                         // discharge to charge
            *g_accChgWH = 0;
            *g_accDsgWH = 0;
            lastState = state;
            state = 1;
            break;
        default:
            break;
    
    }
    
    printf("sigChg:%f,  sigDsg:%f, accChgWH:%f, accDsgWH:%f\n",*g_sigChgWH, *g_sigDsgWH, *g_accChgWH, *g_accDsgWH);


}
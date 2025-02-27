#include <stdint.h>
#include <stdio.h>
#include "sox_private.h"
#include "sox.h"
#include "port.h"
#include "sox_config.h"
int8_t soe_init()
{
    // read saved soe (last saved soe before poweroff)
    if(g_accChgWH && g_accDsgWH)    // if g_accChgWH or g_accDsgWH is NULL, disable this module function
    {
        int8_t ret = read_saved_soe(g_accChgWH, g_accDsgWH);
        if(ret != 0)
        {
            *g_accChgWH = 0;
            *g_accDsgWH = 0;
        }else{
            if(*g_accChgWH < 0){
                *g_accChgWH = 0;
            }
            if(*g_accDsgWH < 0){
                *g_accDsgWH = 0;
            }
        }
    }
    return 0;

}







static float s_lastCur = 0;
int8_t soe_task()
{

    port_soe_input();

    if(g_accChgWH && g_accDsgWH && g_sigChgWH && g_sigDsgWH){
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
    }

    port_soe_output();
    // printf("sigChg:%f,  sigDsg:%f, accChgWH:%f, accDsgWH:%f\n",*g_sigChgWH, *g_sigDsgWH, *g_accChgWH, *g_accDsgWH);
    return 0;

}

void soe_save(bool force)
{
    if(g_accChgWH && g_accDsgWH){
        if(!force){
            static float last_accDsgWH = 0;
            static float last_accChgWH = 0;
            static bool save_flag = false;
            static uint32_t last_time = 0;
            if(*g_accChgWH - last_accChgWH > SOE_SAVE_DIFF_WH || *g_accDsgWH - last_accDsgWH > SOE_SAVE_DIFF_WH)
            {
                save_flag = true;
            }
            if(timebase_get_time_s() - last_time > SOE_SAVE_INTERVAL_S)
            {
                save_flag = false;
                write_saved_soe(*g_accChgWH, *g_accDsgWH);
            } 
        }else{
            write_saved_soe(*g_accChgWH, *g_accDsgWH);
        }

    }
}
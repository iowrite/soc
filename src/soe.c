#include <stdint.h>
#include <math.h>
#include "sox_private.h"
#include "sox.h"
#include "port.h"
#include "sox_config.h"
#include "soe.h"
#include "common.h"


int8_t soe_init()
{
    // read saved soe (last saved soe before poweroff)

    int8_t ret = read_saved_soe(&g_accChgWH, &g_accDsgWH, &g_accChgAH, &g_accDsgAH);
    if(ret != 0)
    {
        g_accChgWH = 0;
        g_accDsgWH = 0;
        g_accChgAH = 0;
        g_accDsgAH = 0;
    }else{
        if(g_accChgWH < 0){
            g_accChgWH = 0;
        }
        if(g_accDsgWH < 0){
            g_accDsgWH = 0;
        }
        if(g_accChgAH < 0){
            g_accChgAH = 0;
        }
        if(g_accDsgAH < 0){
            g_accDsgAH = 0;
        }
    }
    
	
	
	soe_save(FORCE_SAVE);
    return 0;

}







// static float s_lastCur = 0;
/**
 * @brief soe task, update accChgWH, accDsgWH, sigChgWH, sigDsgWH
 * @todo calculate accChgAH, accDsgAH
 */
int8_t soe_task()
{

    if(g_accChgWH && g_accDsgWH && g_sigChgWH && g_sigDsgWH){
        if(g_cur > 0){
            g_accChgWH += g_cur * g_grpVol /3600;
        }else{
            g_accDsgWH += fabsf(g_cur * g_grpVol /3600);
        }
        static int state = 0, lastState = 0;
        UNUSED(lastState);                                  // this function is impleted not good, it need a better implementation                                  
        
        switch(state){
            case 0:                                         // standby
				g_sigChgWH = 0;
                g_sigDsgWH = 0;
                lastState = state;
                if(g_cur > CUR_WINDOW_A)
                {

                    state = 1;
                }else if(g_cur < -CUR_WINDOW_A)
                {
                    state = 3;
                }
                
                break;
            case 1:                                         // charge
                lastState = state;
                if(g_cur> CUR_WINDOW_A)
                {
                    g_sigChgWH += g_cur * g_grpVol /3600;
                }else if(g_cur < -CUR_WINDOW_A)
                {
                    state = 2;
                }else{
					state = 0;
				}
                break;
            case 2:                                         // charge to discharge
                g_sigChgWH = 0;
                g_sigDsgWH = 0;
                lastState = state;
                state = 3;
                break;
            case 3:                                         // discharge        
                lastState = state;
                if(g_cur < -CUR_WINDOW_A){
                    g_sigDsgWH += fabsf(g_cur * g_grpVol /3600);
                }else if(g_cur > CUR_WINDOW_A)
                {
                    state = 4;
                }else{
					state = 0;
				}
                break;
            case 4:                                         // discharge to charge
                g_sigChgWH = 0;
                g_sigDsgWH = 0;
                lastState = state;
                state = 1;
                break;
            default:
                break;
        
        }
    }

    // printf("sigChg:%f,  sigDsg:%f, accChgWH:%f, accDsgWH:%f\n",*g_sigChgWH, *g_sigDsgWH, *g_accChgWH, *g_accDsgWH);
    return 0;

}



/**
 * @brief Save the accumulated charge and discharge energy
 * @param force Force save the soe data or not
 * @return void
 */
void soe_save(bool force)
{
    if(!force){
        static float last_accDsgWH = 0;
        static float last_accChgWH = 0;
        static bool save_flag = false;
        UNUSED(save_flag);
        static uint32_t last_time = 0;
        if(g_accChgWH - last_accChgWH > SOE_SAVE_DIFF_WH || g_accDsgWH - last_accDsgWH > SOE_SAVE_DIFF_WH)
        {
            save_flag = true;
        }
        if(timebase_get_time_s() - last_time > SOE_SAVE_INTERVAL_S)
        {
            save_flag = false;
            write_saved_soe(g_accChgWH, g_accDsgWH, g_accChgAH, g_accDsgAH);
        } 
    }else{
        write_saved_soe(g_accChgWH, g_accDsgWH, g_accChgAH, g_accDsgAH);
    }
}
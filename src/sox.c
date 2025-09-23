#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "sox_private.h"
#include "sox_config.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sox.h"
#include "port.h"


/*******************************************************************************
 * global varible instant(sox module internal use only)
 ******************************************************************************/
uint32_t     g_sox_version =    EKF_SOX_VERSION_MAJOR<<16 
                              | EKF_SOX_VERSION_MINOR<<8
                              | EKF_SOX_VERSION_DEBUG;
float        g_cur;                         // A          
float        g_grpVol;                      // v      
uint16_t     g_celVol[CELL_NUMS];;          // mv                   pointer may be better(small size but can't promise origin data be changed by other module)
int16_t      g_celTmp[CELL_NUMS];;          // dregree              pointer may be better(small size but can't promise origin data be changed by other module)
float        g_celSOC[CELL_NUMS];           // %         
float        g_grpSOC;                      // %       
float        g_celSOH[CELL_NUMS];           // %        
float        g_grpSOH;                      // %            
float        g_cycleCount;                  // times
float        g_accChgWH;                    // WH          
float        g_accDsgWH;                    // WH           
float        g_sigChgWH;                    // WH           
float        g_sigDsgWH;                    // WH     
float        g_accChgAH;                    // AH          
float        g_accDsgAH;                    // AH      
uint16_t    *g_chg_stop_vol;                // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)     
uint16_t    *g_dsg_stop_vol;                // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)   
bool         g_empty;            
bool         g_full;                

uint32_t     g_task_runtime;
uint32_t     g_task_call_tick;

uint8_t      g_soh_calibrate_tigger;          // soh calibration tigger, 0:not trigged, 1:trigged by charging, 2:trigged by discharging
float        g_group_soc_before_jump;

int          g_remain_power_capability;
int          g_remain_energy_conv_eff;
int          g_evolu_of_selfdsg;
int          g_deep_chg_cnt; 
int          g_deep_dsg_cnt; 
uint32_t     g_extreme_chg_time_cnt;
uint32_t     g_extreme_time_cnt;



int8_t sox_init(struct SOX_Init_Attr *attr)
{
    g_chg_stop_vol  = attr->chg_stop_vol;
    g_dsg_stop_vol  = attr->dsg_stop_vol;
    soc_init();
    soh_init();
    soe_init();

    return 0;
} 



static void sox_input(const struct SOX_Input *input)
{
    g_cur = input->cur;
    memcpy(g_celVol, input->vol, sizeof g_celVol);
    memcpy(g_celTmp, input->tmp, sizeof g_celTmp);
    g_grpVol = input->grpVol;
    g_empty  = input->empty;
    g_full   = input->full;

}

int8_t sox_task(const struct SOX_Input *input)
{
#if SOX_DEBUG
    uint32_t start_tick = g_task_call_tick = get_cpu_tick();   // get start cpu tick
#endif
    // input
    sox_input(input);

    // calculate
    soc_task(g_full, g_empty);
    soh_task();
    soe_task();


    // Periodic storage to prevent power outages
    soc_save(AUTO_SAVE);                 // cell soc and group soc
    soh_save(AUTO_SAVE);                 // cell soh , group soh and cycle count
    soe_save(AUTO_SAVE);                 // accumulate charge and discharge energy
	
    // save once, if the current is less than CUR_WINDOW_A, it means that the system is in standby state
	static uint32_t standby_save = 0;
	if(fabsf(g_cur)< CUR_WINDOW_A)
	{
		standby_save++;
		if(standby_save == 1){
			soc_save(FORCE_SAVE);                 
			soh_save(FORCE_SAVE);            
			soe_save(FORCE_SAVE); 
		}
	}else{
		standby_save = 0;
	}
#if SOX_DEBUG
    uint32_t end_tick = get_cpu_tick();     // get end cpu tick
    g_task_runtime = end_tick - start_tick; // calculate task runtime
#endif

    return 0;
}


int8_t sox_manual_set_soc(float soc)
{
    for(int i = 0; i < CELL_NUMS; i++){
        g_socInfo[i].soc = soc;
        g_celSOC[i] = soc;
		if(g_socInfo[i].soc_smooth > 0)
		{
			g_socInfo[i].soc_smooth = soc;
		}
    }
    g_grpSOC = (uint16_t)soc;

    soc_save(true);
    return 0;
}

int8_t sox_manual_set_soh(float soh, uint32_t cycleCount)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        g_celSOH[i] = soh;
    }
    g_grpSOH = soh;
    g_cycleCount = (float)cycleCount;

    soh_save(true);
    return 0;
}



int8_t sox_manual_set_acc_chg_dsg(float accChgWH, float accDsgWH)
{
    g_accChgWH = accChgWH;
    g_accDsgWH = accDsgWH;

    soe_save(true);
    return 0;
}



/*******************************************************************************
 * output functions
 *******************************************************************************/
 
float get_cell_soc(uint16_t cellIndex)
{
    return g_celSOC[cellIndex];
}
float get_group_soc(void)
{
    return g_grpSOC;
}
float get_cell_soh(uint16_t cellIndex)
{
    return g_celSOH[cellIndex];
}
float get_group_soh(void)
{
    return g_grpSOH;
}
float get_cycle_count(void)
{
    return g_cycleCount;
}
float get_sig_chg_wh(void)
{
    return g_sigChgWH;
}
float get_sig_dsg_wh(void)
{
    return g_sigDsgWH;
}
float get_acc_chg_wh(void)
{
    return g_accChgWH;
}
float get_acc_dsg_wh(void)
{
    return g_accDsgWH;
}
float get_acc_chg_ah(void)
{
    return g_accChgAH;
}
float get_acc_dsg_ah(void)
{
    return g_accDsgAH;
}

int8_t get_cell_soc_ary(float *socAry)                                          // fix len: CELL_NUMS
{
    memcpy(socAry, g_celSOC, sizeof(g_celSOC));
    return 0;
}

int8_t get_cell_soh_ary(float *sohAry)                                          // fix len: CELL_NUMS
{
    memcpy(sohAry, g_celSOH, sizeof(g_celSOH));
    return 0;
}


uint32_t get_task_runtime(void)
{
    return g_task_runtime;
}

uint32_t get_task_calltick(void)
{
    return g_task_call_tick;
}


// battery law
int get_num_of_deep_chg(void)
{

    return g_deep_chg_cnt;

}

uint32_t get_time_of_extreme_chg(void)
{
    return g_extreme_chg_time_cnt;
}

int get_num_of_deep_dsg(void)
{
    return g_deep_dsg_cnt;
}

int get_evolu_of_selfdsg(void)
{
    return g_evolu_of_selfdsg;
}
int get_remain_power_capability(void)
{
    return g_remain_power_capability;
}
int get_remain_energy_conv_eff(void)
{
#if SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N == 1
    return get_remain_energy_conv_eff_m1();
#elif SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N == 2
    return get_remain_energy_conv_eff_m2();
#endif
}

uint32_t get_time_of_extreme_temp(void)
{
    return g_extreme_time_cnt;
}






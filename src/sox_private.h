#include <stdint.h>
#include <stdbool.h>
#include "sox_config.h"

#ifndef _SOX_PRIVATE_H
#define _SOX_PRIVATE_H

/******************************************************************************
 * config check
 ******************************************************************************/

#if CELL_NUMS == 0
    #error "please set proper cell number"
#endif

#if CFG_SOX_CELL_TYPE == 0
    #error "Please select one battery type"
#endif

#if PORT_TYPE == 0
    #error "Please select one port type"
#endif

#if PORT_TYPE == 2 && FULL_STD_CLIB == 1
    #error "microcontroller port type can not use FULL_STD_CLIB"
#endif

#if PORT_TYPE == 1 && PORT_ARM_AC_6 == 1
    #error "linux port type can not use ARM Compiler 6"
#endif

#if PORT_TYPE==1 &&  CFG_SOX_PORT_SIM_PROJECT == 0
    #error "Please select one simulate project in linux port type"  
#endif



/******************************************************************************
 * global varible declaration(sox module internal use only)
 ******************************************************************************/
extern float        g_cur;                         // A          
extern float        g_grpVol;                      // v      
extern uint16_t     g_celVol[CELL_NUMS];;          // mv                   pointer may be better(small size but can't promise origin data be changed by other module)
extern int16_t      g_celTmp[CELL_NUMS];;          // dregree              pointer may be better(small size but can't promise origin data be changed by other module)
extern float        g_celSOC[CELL_NUMS];           // %         
extern float        g_grpSOC;                      // %       
extern float        g_celSOH[CELL_NUMS];           // %        
extern float        g_grpSOH;                      // %            
extern float        g_cycleCount;                  // times
extern float        g_accChgWH;                    // WH          
extern float        g_accDsgWH;                    // WH        
extern float        g_accChgAH;                    // AH          
extern float        g_accDsgAH;                    // AH       
extern float        g_sigChgWH;                    // WH           
extern float        g_sigDsgWH;                    // WH           
extern uint16_t    *g_chg_stop_vol;               // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)     
extern uint16_t    *g_dsg_stop_vol;               // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)   
extern bool         g_empty;            
extern bool         g_full;      
extern uint32_t     g_task_runtime;
extern uint32_t     g_task_call_tick;
extern uint8_t      g_soh_calibrate_tigger;       // soh calibration tigger, 0:not trigged, 1:trigged by charging, 2:trigged by discharging  
extern float        g_group_soc_before_jump;

extern int          g_remain_power_capability;
extern int          g_remain_energy_conv_eff;
extern int          g_evolu_of_selfdsg;
extern int          g_deep_chg_cnt; 
extern int          g_deep_dsg_cnt; 
extern uint32_t     g_extreme_chg_time_cnt;
extern uint32_t     g_extreme_time_cnt;

/*******************************************************************************
 * global macro definition(sox module internal use only)
 *******************************************************************************/
#define FORCE_SAVE  1
#define AUTO_SAVE   0


#define MAX_SOH_LIMIT_PERCENTAGE   100


#endif

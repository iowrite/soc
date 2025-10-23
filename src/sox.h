#include <stdint.h>
#include <stdbool.h>
#include "sox_config.h"
#ifndef _SOX_SOX_H
#define _SOX_SOX_H

/******************************************************************************
 * version 
 ******************************************************************************/    
#define EKF_SOX_VERSION_MAJOR         2
#define EKF_SOX_VERSION_MINOR         3
#define EKF_SOX_VERSION_DEBUG         3
 


/******************************************************************************
 * init function
 ******************************************************************************/
struct SOX_Init_Attr{
    uint16_t     *chg_stop_vol;               // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)     
    uint16_t     *dsg_stop_vol;               // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)   
};

struct SOX_Input{
    float       cur;                       
    uint16_t    vol[CELL_NUMS];
    int16_t     tmp[CELL_NUMS];
    float       grpVol;
    bool        full;
    bool        empty;
};

int8_t sox_init(struct SOX_Init_Attr *attr, const struct SOX_Input *input);



/******************************************************************************
 * main loop function
 ******************************************************************************/

int8_t sox_task(const struct SOX_Input *input);


/******************************************************************************
 * manual set functions
 ******************************************************************************/
int8_t sox_manual_set_soc(float soc);
int8_t sox_manual_set_soh(float soh, uint32_t cycleCount);
int8_t sox_manual_set_acc_chg_dsg(float accChgWH, float accDsgWH);


/******************************************************************************
 * output functions
 ******************************************************************************/
float  get_cell_soc(uint16_t cellIndex);
float  get_group_soc(void);
float  get_cell_soh(uint16_t cellIndex);
float  get_group_soh(void);
int8_t get_cell_soc_ary(float *socAry);             // fix len: CELL_NUMS
int8_t get_cell_soh_ary(float *sohAry);             // fix len: CELL_NUMS
float  get_cycle_count(void);
float  get_sig_chg_wh(void);
float  get_sig_dsg_wh(void);
float  get_acc_chg_wh(void);
float  get_acc_dsg_wh(void);
float  get_acc_chg_ah(void);
float  get_acc_dsg_ah(void);
// battery law
int       get_remain_power_capability(void);
int       get_remain_energy_conv_eff(void);
int       get_evolu_of_selfdsg(void);
int       get_num_of_deep_dsg(void);
int       get_num_of_deep_chg(void);
uint32_t  get_time_of_extreme_temp(void);
uint32_t  get_time_of_extreme_chg(void);




/******************************************************************************
 * debug functions
 ******************************************************************************/
uint32_t get_task_runtime(void);
uint32_t get_task_calltick(void);

#endif


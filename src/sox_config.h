#ifndef SOX_CONFIG_H
#define SOX_CONFIG_H

/***********************
 *  SOX Configuration
 **********************/



/***********************
 *  SOC Configuration
 **********************/
#define CAPACITY_AH                     100             // AH, inital capacity(100% soh, Newly manufactured batteries), unit:AH
#define DIFF_T_SEC                      1               // Ampere-hour integration time step, unit:second 

#define DIFF_T_MSEC_ERR                 50              // ms
#define CAP_ERR_AH                      5.0             // AH          
#define CUR_SAMPLE_ERR_A                1.0             // A
#define VOL_SAMPLE_ERR_MV_1             10              // mv
#define VOL_SAMPLE_ERR_MV_2             5               // mv
#define VOL_SAMPLE_ERR_MV_3             3               // mv
#define SOH_ERR_PERCENT                 5               // %

#define GROUP_STATE_CHANGE_TIME         ((uint32_t)(30*60))         //30mins
#define SOC_SAVE_INTERVAFL              ((uint32_t)10)              //10s


/***********************
 *  SOH Configuration
 **********************/
#define MAX_CYCLE_TIME                  10000
#define REFERENCE_CYCLE_TIME            5000



/***********************
 *  SOE Configuration
 **********************/
#define SOE_SAVE_INTERVAL_S         100     
#define SOE_SAVE_DIFF_WH            5


/***********************
 *  SOP Configuration
 **********************/



#endif



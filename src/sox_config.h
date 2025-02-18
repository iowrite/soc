#ifndef SOX_CONFIG_H
#define SOX_CONFIG_H

/***********************
 *  SOX Configuration
 **********************/
#define CELL_NUMS                       16
#define CUR_WINDOW_A                    0.5             //A


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

#define SOC_SAVE_DIFF_PERCENT           1                           // 1%
#define SOC_GRP_SAVE_DIFF_PERCENT       1                           // 1%

#define PURE_AH_LOCK_CUR_THRESHOLD      25                          // A
#define PURE_AH_LOCK_TEMP_THRESHOLD     100                         // 0.1 dregrees

#define STANDBY_HOLD_TIME               600                         // 600s
#define CELL_VOL_BUFFER_LEN             10
#define CELL_VOL_BUFFER_SAMPLE_TIME_S   10.0                        // 1item a second
#define SOC_SMOOTH_START_POINT_CHG      85
#define SOC_SMOOTH_START_POINT_DSG      15


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



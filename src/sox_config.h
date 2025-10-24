#ifndef SOX_USER_CONFIG_FILE
#define SOX_USER_CONFIG_FILE "sox_user_config.h"
#endif
#include SOX_USER_CONFIG_FILE





#ifndef _SOX_CONFIG_H
#define _SOX_CONFIG_H

// script process start 

/******************************************************************************
 *  SOX Configuration
 ******************************************************************************/
#ifndef CELL_NUMS
    #define CELL_NUMS                       0
#endif

#ifndef CUR_WINDOW_A
    #define CUR_WINDOW_A                    4.0f             //A
#endif

#ifndef SOX_DEBUG
    #define SOX_DEBUG                       0
#endif

#ifndef SOX_DEBUG_TIME
    #define SOX_DEBUG_TIME                  0
#endif

#ifndef SOX_DEBUG_SOC_SAVE
    #define SOX_DEBUG_SOC_SAVE              0
#endif

#ifndef SOX_DEBUG_SOH_SAVE
    #define SOX_DEBUG_SOH_SAVE              0   
#endif

#ifndef SOX_DEBUG_SOE_SAVE
    #define SOX_DEBUG_SOE_SAVE              0
#endif


/******************************************************************************
 *  PORT Configuration
 ******************************************************************************/

#ifndef CFG_SOX_CELL_TYPE                   
    #define CFG_SOX_CELL_TYPE                   0               // 1: 100AH EVE, 2: 314AH RUIPU
#endif

#ifndef PORT_TYPE
    #define PORT_TYPE                           0               // 1 linux, 2 microcontroller
#endif

#ifndef FULL_STD_CLIB
    #define FULL_STD_CLIB       0
#endif

#ifndef CFG_SOX_PORT_SIM_PROJECT
    #define CFG_SOX_PORT_SIM_PROJECT      0                     // 1:stack 2:jiguang 3:314 cell curve
#endif

#ifndef PORT_ARM_AC_6
    #define PORT_ARM_AC_6       0                       // ARM Compiler 6 or ARM Compiler 5, 
#endif

 

/******************************************************************************
 *  SOC Configuration
 ******************************************************************************/


#ifndef DIFF_T_SEC
    #define DIFF_T_SEC                      1               // Ampere-hour integration time step, unit:second 
#endif

#ifndef DIFF_T_MSEC_ERR
    #define DIFF_T_MSEC_ERR                 50              // ms
#endif

#ifndef CAP_ERR_AH
    #define CAP_ERR_AH                      5.0f            // AH          
#endif

#ifndef CUR_SAMPLE_ERR_A
    #define CUR_SAMPLE_ERR_A                1.0f            // A
#endif

#ifndef VOL_SAMPLE_ERR_MV_1
    #define VOL_SAMPLE_ERR_MV_1             50              // mv
#endif

#ifndef VOL_SAMPLE_ERR_MV_2
    #define VOL_SAMPLE_ERR_MV_2             40              // mv
#endif

#ifndef VOL_SAMPLE_ERR_MV_3
    #define VOL_SAMPLE_ERR_MV_3             20              // mv
#endif

#ifndef VOL_SAMPLE_ERR_MV_4
    #define VOL_SAMPLE_ERR_MV_4             15              // mv
#endif

#ifndef VOL_SAMPLE_ERR_MV_1_H
    #define VOL_SAMPLE_ERR_MV_1_H           1.5f
#endif

#ifndef VOL_SAMPLE_ERR_MV_4_H
    #define VOL_SAMPLE_ERR_MV_4_H           4.0f
#endif

#ifndef SOH_ERR_PERCENT
    #define SOH_ERR_PERCENT                 5               // %
#endif

#ifndef EKF_Q_K
    #define EKF_Q_K                         1
#endif

#ifndef GROUP_STATE_CHANGE_TIME
    #define GROUP_STATE_CHANGE_TIME         ((uint32_t)(30*60))         //30mins
#endif

#ifndef SOC_SAVE_INTERVAFL
    #define SOC_SAVE_INTERVAFL              ((uint32_t)10)              //10s
#endif

#ifndef SOC_SAVE_DIFF_PERCENT
    #define SOC_SAVE_DIFF_PERCENT           1                           // 1%
#endif

#ifndef SOC_GRP_SAVE_DIFF_PERCENT
    #define SOC_GRP_SAVE_DIFF_PERCENT       1                           // 1%
#endif

#ifndef PURE_AH_LOCK_CUR_THRESHOLD
    #define PURE_AH_LOCK_CUR_THRESHOLD      25                          // A
#endif

#ifndef PURE_AH_LOCK_TEMP_THRESHOLD
    #define PURE_AH_LOCK_TEMP_THRESHOLD     100                         // 0.1 dregrees
#endif

#ifndef STANDBY_HOLD_TIME
    #define STANDBY_HOLD_TIME               ((uint32_t)600)                         // 600s
#endif

#ifndef CELL_VOL_BUFFER_LEN
    #define CELL_VOL_BUFFER_LEN             (10+1)                      // 10s(time span)                    
#endif

#ifndef CELL_VOL_BUFFER_SAMPLE_TIME_S
    #define CELL_VOL_BUFFER_SAMPLE_TIME_S   6                            // 1item per 6 second
#endif

#ifndef SOC_SMOOTH_START_POINT_CHG
    #define SOC_SMOOTH_START_POINT_CHG      85
#endif

#ifndef SOC_SMOOTH_START_POINT_DSG
    #define SOC_SMOOTH_START_POINT_DSG      30
#endif

#ifndef SOC_SMOOTH_START_VOL_DSG
    #define SOC_SMOOTH_START_VOL_DSG        3050
#endif

#ifndef SOC_SMOOTH_START_VOL_CHG
    #define SOC_SMOOTH_START_VOL_CHG        3400
#endif

#ifndef MAX_EKF_Q_PERCENT
    #define MAX_EKF_Q_PERCENT               5      // 10%
#endif

#ifndef SOC_INIT_OCV_CAL_ENABLE
    #define SOC_INIT_OCV_CAL_ENABLE          0
#endif

#ifndef SOC_INIT_OCV_CAL_DIFF_THRESHOLD
    #define SOC_INIT_OCV_CAL_DIFF_THRESHOLD  30
#endif

#ifndef SOC_TEMPRA_WARM_CAP_OFFSET_1
    #define SOC_TEMPRA_WARM_CAP_OFFSET_1      1.0f
#endif

#ifndef SOC_TEMPRA_WARM_CAP_OFFSET_2
    #define SOC_TEMPRA_WARM_CAP_OFFSET_2      0.5f
#endif

#ifndef SOC_TEMPRA_WARM_CAP_OFFSET_3
    #define SOC_TEMPRA_WARM_CAP_OFFSET_3      0
#endif

#ifndef SOC_MAX_CALCULATE_VALUE
    #define SOC_MAX_CALCULATE_VALUE          99
#endif

#ifndef SOC_MIN_CALCULATE_VALUE
    #define SOC_MIN_CALCULATE_VALUE          1
#endif

#ifndef SOX_GROUP_FULL_CAL_CELL
    #define SOX_GROUP_FULL_CAL_CELL         1                                       // set all cell to 100% when group is full(one cell is full)
#endif

#ifndef SOX_GROUP_EMPTY_CAL_CELL
    #define SOX_GROUP_EMPTY_CAL_CELL        1                                       // set all cell to 0% when group is empty(one cell is empty)     
#endif

#ifndef SOC_FAKE_SMOOTH_ENABLE
    #define SOC_FAKE_SMOOTH_ENABLE         0                                       // fake smooth function
#endif

/******************************************************************************
 *  SOH Configuration
 ******************************************************************************/
#ifndef SOX_CFG_H_SAVE_CHECK_TIME
    #define SOX_CFG_H_SAVE_CHECK_TIME   (uint32_t)(60*60*24)    
#endif

#ifndef MAX_CYCLE_TIME
    #define MAX_CYCLE_TIME                  10000                      // for soh init(limitation of cycle count)
#endif

#ifndef REFERENCE_CYCLE_TIME
    #define REFERENCE_CYCLE_TIME            5000                       //
#endif

#ifndef SOH_HIGH_TEMP
    #define SOH_HIGH_TEMP             450                              // degree 45
#endif

#ifndef SOH_LOW_TEMP
    #define SOH_LOW_TEMP              250                              // degree 25
#endif

#ifndef SOH_CYCLE_L1_PERCENT
    #define SOH_CYCLE_L1_PERCENT                       80.0f
#endif

#ifndef SOH_HIGH_TEMP_CYCLE_L1
    #define SOH_HIGH_TEMP_CYCLE_L1                      2000             // soh 100% decay to SOH_CYCLE_L1_PERCENT%
#endif

#ifndef SOH_LOW_TEMP_CYCLE_L1
    #define SOH_LOW_TEMP_CYCLE_L1                       5000             // soh 100% decay to SOH_CYCLE_L1_PERCENT%
#endif

#ifndef SOH_PASSIVE_CALIBRATE_TEMP_LIMIT
    #define SOH_PASSIVE_CALIBRATE_TEMP_LIMIT      200               // 20 dregrees, one digits(interger)
#endif

#ifndef SOH_PASSIVE_GRP_SOC_CHG_START
    #define SOH_PASSIVE_GRP_SOC_CHG_START              15
#endif

#ifndef SOH_PASSIVE_GRP_SOC_DSG_START
    #define SOH_PASSIVE_GRP_SOC_DSG_START              90
#endif

#ifndef SOX_CFG_H_BAT_LAW
    #define SOX_CFG_H_BAT_LAW                       0                     // function of batttery law enable/disable
#endif

#ifndef SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N
    #define SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N    2                  // remain_energy_conv_eff calculate method n  (1 or 2)   
#endif


/******************************************************************************
 *  SOE Configuration
 ******************************************************************************/
#ifndef SOE_SAVE_INTERVAL_S
    #define SOE_SAVE_INTERVAL_S         (uint32_t)100         
#endif

#ifndef SOE_SAVE_DIFF_WH
    #define SOE_SAVE_DIFF_WH            5
#endif


/******************************************************************************
 *  SOP Configuration
 ******************************************************************************/





// script process end

#endif
#warning "<this file is generate by default_config_to_user_config.sh>, your are useing default config, please define SOX_USER_CONFIG_FILE to your own config file,or delete this warning, edit this file accord your project"
#ifndef SOX_USER_CONFIG_H
#define SOX_USER_CONFIG_H

/******************************************************************************
 *  SOX Configuration
 ******************************************************************************/
    #define CELL_NUMS                       4

    #define CUR_WINDOW_A                    4.0f             //A

    #define SOX_DEBUG                       0

    #define SOX_DEBUG_TIME                  0

    #define SOX_DEBUG_SOC_SAVE              0

    #define SOX_DEBUG_SOH_SAVE              0   

    #define SOX_DEBUG_SOE_SAVE              0


/******************************************************************************
 *  PORT Configuration
 ******************************************************************************/

    #define CFG_SOX_CELL_TYPE                   0               // 1: 100AH EVE, 2: 314AH RUIPU

    #define PORT_TYPE                           0               // 1 linux, 2 microcontroller

    #define FULL_STD_CLIB       0

    #define CFG_SOX_PORT_SIM_PROJECT      0                     // 1:stack 2:jiguang 3:314 cell curve

    #define PORT_ARM_AC_6       0                       // ARM Compiler 6 or ARM Compiler 5, 

 

/******************************************************************************
 *  SOC Configuration
 ******************************************************************************/


    #define DIFF_T_SEC                      1               // Ampere-hour integration time step, unit:second 

    #define DIFF_T_MSEC_ERR                 50              // ms

    #define CAP_ERR_AH                      5.0f            // AH          

    #define CUR_SAMPLE_ERR_A                1.0f            // A

    #define VOL_SAMPLE_ERR_MV_1             50              // mv

    #define VOL_SAMPLE_ERR_MV_2             40              // mv

    #define VOL_SAMPLE_ERR_MV_3             20              // mv

    #define VOL_SAMPLE_ERR_MV_4             15              // mv

    #define VOL_SAMPLE_ERR_MV_1_H           1.5f

    #define VOL_SAMPLE_ERR_MV_4_H           4.0f

    #define SOH_ERR_PERCENT                 5               // %

    #define EKF_Q_K                         1

    #define GROUP_STATE_CHANGE_TIME         ((uint32_t)(30*60))         //30mins

    #define SOC_SAVE_INTERVAFL              ((uint32_t)10)              //10s

    #define SOC_SAVE_DIFF_PERCENT           1                           // 1%

    #define SOC_GRP_SAVE_DIFF_PERCENT       1                           // 1%

    #define PURE_AH_LOCK_CUR_THRESHOLD      25                          // A

    #define PURE_AH_LOCK_TEMP_THRESHOLD     100                         // 0.1 dregrees

    #define STANDBY_HOLD_TIME               ((uint32_t)600)                         // 600s

    #define CELL_VOL_BUFFER_LEN             (10+1)                      // 10s(time span)                    

    #define CELL_VOL_BUFFER_SAMPLE_TIME_S   6                            // 1item per 6 second

    #define SOC_SMOOTH_START_POINT_CHG      85

    #define SOC_SMOOTH_START_POINT_DSG      30

    #define SOC_SMOOTH_START_VOL_DSG        3050

    #define SOC_SMOOTH_START_VOL_CHG        3400

    #define MAX_EKF_Q_PERCENT               5      // 10%

    #define SOC_INIT_OCV_CAL_ENABLE          0

    #define SOC_INIT_OCV_CAL_DIFF_THRESHOLD  30

    #define SOC_TEMPRA_WARM_CAP_OFFSET_1      1.0f

    #define SOC_TEMPRA_WARM_CAP_OFFSET_2      0.5f

    #define SOC_TEMPRA_WARM_CAP_OFFSET_3      0

    #define SOC_MAX_CALCULATE_VALUE          99

    #define SOC_MIN_CALCULATE_VALUE          1

    #define SOX_GROUP_FULL_CAL_CELL         1                                       // set all cell to 100% when group is full(one cell is full)

    #define SOX_GROUP_EMPTY_CAL_CELL        1                                       // set all cell to 0% when group is empty(one cell is empty)     

    #define SOC_FAKE_SMOOTH_ENABLE         0                                       // fake smooth function

/******************************************************************************
 *  SOH Configuration
 ******************************************************************************/
    #define SOX_CFG_H_SAVE_CHECK_TIME   (uint32_t)(60*60*24)    

    #define MAX_CYCLE_TIME                  10000                      // for soh init(limitation of cycle count)

    #define REFERENCE_CYCLE_TIME            5000                       //

    #define SOH_HIGH_TEMP             450                              // degree 45

    #define SOH_LOW_TEMP              250                              // degree 25

    #define SOH_CYCLE_L1_PERCENT                       80.0f

    #define SOH_HIGH_TEMP_CYCLE_L1                      2000             // soh 100% decay to SOH_CYCLE_L1_PERCENT%

    #define SOH_LOW_TEMP_CYCLE_L1                       5000             // soh 100% decay to SOH_CYCLE_L1_PERCENT%

    #define SOH_PASSIVE_CALIBRATE_TEMP_LIMIT      200               // 20 dregrees, one digits(interger)

    #define SOH_PASSIVE_GRP_SOC_CHG_START              15

    #define SOH_PASSIVE_GRP_SOC_DSG_START              90

    #define SOX_CFG_H_BAT_LAW                       0                     // function of batttery law enable/disable

    #define SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N    2                  // remain_energy_conv_eff calculate method n  (1 or 2)   


/******************************************************************************
 *  SOE Configuration
 ******************************************************************************/
    #define SOE_SAVE_INTERVAL_S         (uint32_t)100         

    #define SOE_SAVE_DIFF_WH            5


/******************************************************************************
 *  SOP Configuration
 ******************************************************************************/





#endif

#ifndef _SOX_CONFIG_H
#define _SOX_CONFIG_H

/***********************
 *  SOX Configuration
 **********************/
#define CELL_NUMS                       16
#define CUR_WINDOW_A                    4.0f             //A


/***********************
 *  SOC Configuration
 **********************/
#define CAPACITY_AH                     100             // AH, inital capacity(100% soh, Newly manufactured batteries), unit:AH
#define DIFF_T_SEC                      1               // Ampere-hour integration time step, unit:second 

#define DIFF_T_MSEC_ERR                 50              // ms
#define CAP_ERR_AH                      5.0             // AH          
#define CUR_SAMPLE_ERR_A                1.0             // A
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



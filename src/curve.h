#include "sox_config.h"

#include <stdint.h>

#ifndef _SOX_CURVE_H
#define _SOX_CURVE_H

#define TEMP_POINT_NUM              8      // -15  -5  5  15 25   35  45  55   
#define CUR_POINT_NUM               5       // 0.1 0.2 0.3 0.4 0.5
#define SOC_POINT_NUM               29
#define OCV_POINT_NUM               21


extern const uint16_t s_cap_list_chg[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const uint16_t s_cap_list_dsg[TEMP_POINT_NUM][CUR_POINT_NUM];

extern const uint16_t v_25d5c_chg[SOC_POINT_NUM];
extern const int16_t k_25d5c_chg[SOC_POINT_NUM];
extern const uint16_t v_25d5c_dsg[SOC_POINT_NUM];
extern const int16_t k_25d5c_dsg[SOC_POINT_NUM];

extern const uint16_t v_25d3c_chg[SOC_POINT_NUM];
extern const int16_t k_25d3c_chg[SOC_POINT_NUM];


extern const uint16_t* const s_chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const int16_t* const s_chg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const uint16_t* const s_dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const int16_t* const s_dsg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM];

extern const uint16_t * temp_ocv[TEMP_POINT_NUM];


#if SOX_CFG_H_BAT_LAW

// unit kw, interval 100 cycles                                         
extern const int remain_power_capability_table[6];

// unit %, interval 6 month                                        
extern const int remain_self_dsg_table[6];

extern const int energy_convert_efficiency_table[6];

#endif


#endif 



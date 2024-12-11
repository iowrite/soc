#include <stdint.h>

#ifndef __CURVE_H
#define __CURVE_H

#define TEMP_POINT_NUM              7      // 0 5  15 25   35  45  55   
#define CUR_POINT_NUM               5       // 0.1 0.2 0.3 0.4 0.5
#define SOC_POINT_NUM               29


extern const uint16_t s_cap_list_chg[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const uint16_t s_cap_list_dsg[TEMP_POINT_NUM][CUR_POINT_NUM];

extern const uint16_t v_25d5c_chg[SOC_POINT_NUM];
extern const int16_t k_25d5c_chg[SOC_POINT_NUM];
extern const uint16_t v_25d5c_dsg[SOC_POINT_NUM];
extern const int16_t k_25d5c_dsg[SOC_POINT_NUM];

extern const uint16_t v_25d3c_chg[SOC_POINT_NUM];
extern const int16_t k_25d3c_chg[SOC_POINT_NUM];


extern const uint16_t* s_chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const int16_t* s_chg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const uint16_t* s_dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM];
extern const int16_t* s_dsg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM];




#endif 



#include <stdint.h>

#ifndef _SOX_SOH_H
#define _SOX_SOH_H

#define SOH_CALIBRATION_TIGGERED_BY_CHARGING    1
#define SOH_CALIBRATION_TIGGERED_BY_DISCHARGING 2


int8_t soh_init(void);
int8_t soh_task(void);
void soh_save(bool force);

int get_remain_energy_conv_eff_m1(void);
int get_remain_energy_conv_eff_m2(void);

#endif
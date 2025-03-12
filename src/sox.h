#include <stdint.h>
#include <stdbool.h>
#ifndef _SOX_SOX_H
#define _SOX_SOX_H

int8_t sox_init( 
    float *cur, 
    uint16_t *vol, 
    int16_t *tmp, 
    uint16_t *soc, 
    uint16_t *grpSOC , 
    float *soh, float *grpSOH, 
    uint32_t *cycleCount,
    float *grpVol,
    float *sigChgWH,
    float *sigDsgWH,
    float *accChgWH,
    float *accDsgWH,
    uint16_t *g_chg_stop_vol,
    uint16_t *g_dsg_stop_vol);
int8_t sox_task(bool full, bool empty);

int8_t sox_manual_set_soc(float soc);
int8_t sox_manual_set_soh(float soh, uint32_t cycleCount);
int8_t sox_manual_set_acc_chg_dsg(float accChgWH, float accDsgWH);


#endif


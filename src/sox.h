#include <stdint.h>
#include <stdbool.h>
#ifndef __SOX_H__
#define __SOX_H__

#define CELL_NUMS                       16

#define CUR_WINDOW_A                    0.5             //A













int8_t sox_init( 
    float *cur, 
    uint16_t *vol, 
    int16_t *tmp, 
    uint16_t *soc, 
    uint16_t *grpSOC , 
    double *soh, double *grpSOH, 
    uint32_t *cycleCount,
    float *grpVol,
    float *sigChgWH,
    float *sigDsgWH,
    float *accChgAH,
    float *accDsgAH);
int8_t sox_task(bool full, bool empty);






#endif


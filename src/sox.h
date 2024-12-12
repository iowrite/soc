#include <stdint.h>
#include <stdbool.h>
#ifndef __SOX_H__
#define __SOX_H__

#define CELL_NUMS                   16





int8_t sox_init( float *cur, uint16_t *vol, int16_t *tmp, uint16_t *soc, uint16_t *grpSOC , float *soh, float *grpSOH, uint16_t *cycleCount);
int8_t sox_task(bool full, bool empty);
#endif


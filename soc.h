#include <stdint.h>
#include <stdbool.h>

#ifndef _SOC_H
#define _SOC_H



void SOC_Init(float *cur, uint16_t *vol, uint16_t *tmp, uint16_t *soc, uint16_t *grpSOC);
void SOC_Task(bool full, bool empty);


#endif


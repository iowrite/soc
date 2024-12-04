#include <stdint.h>


#ifndef _SOC_H
#define _SOC_H


#define DIFF_T_MSEC                  1           //second
#define CAPACITY_AH                 100         //AH
#define CUR_SAMPLE_ERR_A            1.0         //A
#define VOL_SAMPLE_ERR_MV           10          //mv
#define CUR_WINDOW_A                0.5         //A


#define CELL_NUMS                   16          








void SOC_Init(float *cur, uint16_t *vol, uint16_t *tmp, uint16_t *soc, uint16_t *grpSOC);
void SOC_Task(void);


#endif


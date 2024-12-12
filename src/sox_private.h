#include <stdint.h>
#ifndef _SOX_PRIVATE_H_
#define _SOX_PRIVATE_H_

// input 
extern float *g_cur;
extern uint16_t *g_celVol;
extern int16_t *g_celTmp;
// output
extern uint16_t *g_celSOC;
extern uint16_t *g_grpSOC;
extern float *g_celSOH;
extern float *g_grpSOH;

extern uint16_t *g_cycleCount;


#endif
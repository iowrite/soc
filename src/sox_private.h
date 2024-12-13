#include <stdint.h>
#ifndef _SOX_PRIVATE_H_
#define _SOX_PRIVATE_H_

extern float *g_cur;                       // A            *1
extern float *g_grpVol;                    // vol          *1 
extern uint16_t *g_celVol;                 // mv           *1
extern int16_t *g_celTmp;                  // dregree      *10
extern uint16_t *g_celSOC;                 // %            *10
extern uint16_t *g_grpSOC;                 // %            *10
extern double  *g_celSOH;                  // %            *1
extern double  *g_grpSOH;                  // %            *1
extern uint32_t *g_cycleCount;             // times        *1000
extern float *g_accChgWH;                // AH           *1
extern float *g_accDsgWH;                // AH           *1
extern float *g_sigChgWH;                // AH           *1
extern float *g_sigDsgWH;                // AH           *1

#endif
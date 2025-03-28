#include <stdint.h>
#ifndef _SOX_PRIVATE_H
#define _SOX_PRIVATE_H

extern float *g_cur;                        // A            *1
extern float *g_grpVol;                     // vol          *1 
extern uint16_t *g_celVol;                  // mv           *1
extern int16_t *g_celTmp;                   // dregree      *10
extern uint16_t *g_celSOC;                  // %            *10
extern uint16_t *g_grpSOC;                  // %            *10
extern float  *g_celSOH;                   // %            *1
extern float  *g_grpSOH;                   // %            *1
extern uint32_t *g_cycleCount;              // times        *1000
extern float *g_accChgWH;                   // WH           *1
extern float *g_accDsgWH;                   // WH           *1
extern float *g_sigChgWH;                   // WH           *1
extern float *g_sigDsgWH;                   // WH           *1
extern uint16_t *g_chg_stop_vol;            // mv           *1
extern uint16_t *g_dsg_stop_vol;            // mv           *1
#endif
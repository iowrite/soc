#include <stdint.h>
#include <stdbool.h>
#include "sox_config.h"

#ifndef _SOX_PRIVATE_H
#define _SOX_PRIVATE_H


/*******************************************************************************
 * global varible declaration(sox module internal use only)
 *******************************************************************************/
extern float        g_cur;                         // A          
extern float        g_grpVol;                      // v      
extern uint16_t     g_celVol[CELL_NUMS];;          // mv                   pointer may be better(small size but can't promise origin data be changed by other module)
extern int16_t      g_celTmp[CELL_NUMS];;          // dregree              pointer may be better(small size but can't promise origin data be changed by other module)
extern float        g_celSOC[CELL_NUMS];           // %         
extern float        g_grpSOC;                      // %       
extern float        g_celSOH[CELL_NUMS];           // %        
extern float        g_grpSOH;                      // %            
extern float        g_cycleCount;                  // times
extern float        g_accChgWH;                    // WH          
extern float        g_accDsgWH;                    // WH        
extern float        g_accChgAH;                    // AH          
extern float        g_accDsgAH;                    // AH       
extern float        g_sigChgWH;                    // WH           
extern float        g_sigDsgWH;                    // WH           
extern uint16_t    *g_chg_stop_vol;               // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)     
extern uint16_t    *g_dsg_stop_vol;               // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)   
extern bool         g_empty;            
extern bool         g_full;      
extern uint32_t     g_task_runtime;
extern uint32_t     g_task_call_tick;

/*******************************************************************************
 * global macro definition(sox module internal use only)
 *******************************************************************************/
#define FORCE_SAVE  1
#define AUTO_SAVE   0


#endif
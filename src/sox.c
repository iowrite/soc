#include <stdint.h>
#include <stdbool.h>
#include "sox_private.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sop.h"
#include "sox.h"



float *g_cur;                       // A            *1
float *g_grpVol;                    // v            *1
uint16_t *g_celVol;                 // mv           *1
int16_t *g_celTmp;                  // dregree      *10
uint16_t *g_celSOC;                 // %            *10
uint16_t *g_grpSOC;                 // %            *10
double  *g_celSOH;                  // %            *1
double  *g_grpSOH;                  // %            *1
uint32_t *g_cycleCount;             // times        *1000
float *g_accChgWH;                  // WH           *1
float *g_accDsgWH;                  // WH           *1
float *g_sigChgWH;                  // WH           *1
float *g_sigDsgWH;                  // WH           *1
uint16_t *g_chg_stop_vol;           // mv           *1
uint16_t *g_dsg_stop_vol;           // mv           *1




int8_t sox_init( 
    float *cur,                         // input only
    uint16_t *vol,                      // input only
    int16_t *tmp,                       // input only
    uint16_t *soc,                      // input and output
    uint16_t *grpSOC ,                  // input and output
    double *soh,                        // input and output
    double *grpSOH,                     // input and output
    uint32_t *cycleCount,               // input and output
    float *grpVol,                      // input only
    float *sigChgWH,                    // input and output
    float *sigDsgWH,                    // input and output
    float *accChgAH,                    // input and output
    float *accDsgAH,                    // input and output
    uint16_t *chg_stop_vol,             // input only
    uint16_t *dsg_stop_vol              // input only
    )
{
    g_cur = cur;
    g_celVol = vol;
    g_celTmp = tmp;
    g_celSOC = soc;
    g_grpSOC = grpSOC;
    g_celSOH = soh;
    g_grpSOH = grpSOH;
    g_cycleCount = cycleCount;
    g_grpVol = grpVol;
    g_sigChgWH = sigChgWH;
    g_sigDsgWH = sigDsgWH;
    g_accChgWH = accChgAH;
    g_accDsgWH = accDsgAH;
    g_chg_stop_vol = chg_stop_vol;
    g_dsg_stop_vol = dsg_stop_vol;
    soc_init();
    soh_init();
    soe_init();



    return 0;
} 



int8_t sox_task(bool full, bool empty)
{
    // calculate
    soc_task(full, empty);
    soh_task();
    soe_task();

    // Periodic storage to prevent power outages
    soc_save();                 // cell soc and group soc
    soh_save();                 // cell soh , group soh and cycle count
    soe_save();                 // accumulate charge and discharge energy
     
    return 0;
}
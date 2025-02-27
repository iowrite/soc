#include <stdint.h>
#include <stdbool.h>
#include "sox_private.h"
#include "sox_config.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sop.h"
#include "sox.h"
#include "port.h"



float *g_cur;                       // A            *1
float *g_grpVol;                    // v            *1
uint16_t *g_celVol;                 // mv           *1
int16_t *g_celTmp;                  // dregree      *10
uint16_t *g_celSOC;                 // %            *10
uint16_t *g_grpSOC;                 // %            *10
float  *g_celSOH;                  // %            *1
float  *g_grpSOH;                  // %            *1
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
    float *soh,                         // input and output
    float *grpSOH,                      // output only
    uint32_t *cycleCount,               // input and output
    float *grpVol,                      // input only
    float *sigChgWH,                    // input and output
    float *sigDsgWH,                    // input and output
    float *accChgWH,                    // input and output
    float *accDsgWH,                    // input and output
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
    g_accChgWH = accChgWH;
    g_accDsgWH = accDsgWH;
    g_chg_stop_vol = chg_stop_vol;
    g_dsg_stop_vol = dsg_stop_vol;
    soc_init();
    soh_init();
    soe_init();



    return 0;
} 



int8_t sox_task(bool full, bool empty)
{
    port_sox_input();
    // calculate
    soc_task(full, empty);
    soh_task();
    soe_task();
    port_sox_output();

    // Periodic storage to prevent power outages
    soc_save(false);                 // cell soc and group soc
    soh_save(false);                 // cell soh , group soh and cycle count
    soe_save(false);                 // accumulate charge and discharge energy
     
    return 0;
}


int8_t sox_manual_set_soc(float soc)
{
    for(int i = 0; i < CELL_NUMS; i++){
        g_socInfo[i].soc = soc;
    }
    *g_grpSOC = (uint16_t)soc;

    port_soc_output();
    soc_save(true);
    return 0;
}

int8_t sox_manual_set_soh(float soh, uint32_t cycleCount)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        g_celSOH[i] = soh;
    }
    *g_grpSOH = soh;
    *g_cycleCount = cycleCount;

    port_soh_output();
    soh_save(true);
    return 0;
}



int8_t sox_manual_set_acc_chg_dsg(float accChgWH, float accDsgWH)
{
    *g_accChgWH = accChgWH;
    *g_accDsgWH = accDsgWH;

    port_soe_output();
    soe_save(true);
}
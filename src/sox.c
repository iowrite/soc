#include <stdint.h>
#include <stdbool.h>
#include "sox_private.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sop.h"
#include "sox.h"



float *g_cur;                       // A            *1
uint16_t *g_celVol;                 // mv           *1
int16_t *g_celTmp;                  // dregree      *10
uint16_t *g_celSOC;                 // %            *10
uint16_t *g_grpSOC;                 // %            *10
double  *g_celSOH;                  // %            *1
double  *g_grpSOH;                  // %            *1
uint32_t *g_cycleCount;             // times        *1000



int8_t sox_init( 
    float *cur, 
    uint16_t *vol, 
    int16_t *tmp, 
    uint16_t *soc, 
    uint16_t *grpSOC , 
    double *soh, 
    double *grpSOH,
    uint32_t *cycleCount)
{
    g_cur = cur;
    g_celVol = vol;
    g_celTmp = tmp;
    g_celSOC = soc;
    g_grpSOC = grpSOC;
    g_celSOH = soh;
    g_grpSOH = grpSOH;
    g_cycleCount = cycleCount;

    soc_init();
    soh_init();



    return 0;
} 



int8_t sox_task(bool full, bool empty)
{
    soc_task(full, empty);
    soh_task();


    return 0;
}
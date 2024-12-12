#include <stdint.h>
#include <stdbool.h>
#include "sox_private.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sop.h"
#include "sox.h"



float *g_cur;
uint16_t *g_celVol;
int16_t *g_celTmp;
uint16_t *g_celSOC;
uint16_t *g_grpSOC;
float  *g_celSOH;
float  *g_grpSOH;
uint16_t *g_cycleCount;



int8_t sox_init( 
    float *cur, 
    uint16_t *vol, 
    int16_t *tmp, 
    uint16_t *soc, 
    uint16_t *grpSOC , 
    float *soh, 
    float *grpSOH,
    uint16_t *cycleCount)
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
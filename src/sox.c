#include <stdint.h>
#include <stdbool.h>
#include "sox_private.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sop.h"





int8_t sox_init( 
    float *cur, 
    uint16_t *vol, 
    uint16_t *tmp, 
    uint16_t *soc, 
    uint16_t *grpSOC , 
    uint16_t *soh, 
    uint16_t *grpSOH)
{
    g_cur = cur;
    g_celVol = vol;
    g_celTmp = tmp;
    g_celSOC = soc;
    g_grpSOC = grpSOC;
    g_celSOH = soh;
    g_grpSOH = grpSOH;

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
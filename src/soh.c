#include <stdint.h>
#include <stddef.h>
#include "sox.h"
#include "sox_private.h"
#include "string.h"

static uint16_t s_lastSOC[CELL_NUMS];
static uint16_t s_lastGrpSOC;
int8_t  soh_init()
{
    memcpy(s_lastSOC, g_celSOC, sizeof(s_lastSOC));
    s_lastGrpSOC = *g_grpSOC;
    return 0;
}


int8_t soh_task()
{
    if(*g_grpSOC < s_lastGrpSOC-10){
        s_lastGrpSOC  = *g_grpSOC;
        *g_cycleCount += 5;
    }else if(*g_grpSOC > s_lastGrpSOC+10){
        s_lastGrpSOC  = *g_grpSOC;
        *g_cycleCount += 5;
    }

    float sumSOH = 0;
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        if (g_celSOC[i] < s_lastSOC[i]-10 || g_celSOC[i] > s_lastSOC[i]+10)
        {
            s_lastSOC[i] = g_celSOC[i];
            if(g_celTmp[i] < 250)
            {
                float subk = 1/5000;
                g_celSOH[i] -= 5*subk;
            }else if(g_celTmp[i] < 450)
            {
                float subk = 1/(g_celTmp[i] - 250)/200.0*3000+2000;
                g_celSOH[i] -= 5*subk;
            }else{
                float subk = 1/2000;
                g_celSOH[i] -= 5*subk;
            }
        }
        sumSOH += g_celSOH[i];
    }
    
    *g_grpSOH = sumSOH/16;


}






#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "sox.h"
#include "sox_private.h"



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
    // bug: lost some cycle when charge change to discharge or discharge to charge
    if(*g_grpSOC <= s_lastGrpSOC-10){
        *g_cycleCount += 5*((s_lastGrpSOC-*g_grpSOC)/10);
        s_lastGrpSOC  = *g_grpSOC;
    }else if(*g_grpSOC >= s_lastGrpSOC+10){
        *g_cycleCount += 5*((*g_grpSOC-s_lastGrpSOC)/10);
        s_lastGrpSOC  = *g_grpSOC;
        
    }

    double sumSOH = 0;
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        if (g_celSOC[i] <= s_lastSOC[i]-10 || g_celSOC[i] >= s_lastSOC[i]+10)
        {
            int delta = abs(g_celSOC[i] - s_lastSOC[i])/10;
            if(g_celTmp[i] <= 250)
            {
                float subk = 20.0/5000;
                g_celSOH[i] -= 0.01*delta*0.5*subk;
            }else if(g_celTmp[i] < 450)
            {
                // printf("11111111111111\n");
                float subk = 20.0/(5000-((g_celTmp[i] - 250)/200.0)*3000);
                g_celSOH[i] -= 0.01*delta*0.5*subk;
            }else{
                // printf("2222222222222222\n");
                float subk = 20.0/2000;
                g_celSOH[i] -= 0.01*delta*0.5*subk;
            }
            s_lastSOC[i] = g_celSOC[i];
        }
        sumSOH += g_celSOH[i];
    }
    
    *g_grpSOH = sumSOH/16;

    // printf("cycle: %d  SOH: %f\n", *g_cycleCount, *g_grpSOH);
}






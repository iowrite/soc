#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "sox.h"
#include "sox_private.h"
#include "port.h"

#define MAX_CYCLE_TIME                  10000
#define REFERENCE_CYCLE_TIME            5000

static uint16_t s_lastSOC[CELL_NUMS];
static uint16_t s_lastGrpSOC;
int8_t  soh_init()
{
    // read last cycle time(saved before last poweroff)
    uint32_t cycleTime;
    uint8_t ret = read_saved_cycle(&cycleTime);
    if(ret == 0){
        if(cycleTime == 0xffffffff){    // first powerup(empty eeprom or flash)
            cycleTime = 0;
        }else if(cycleTime>MAX_CYCLE_TIME)
        {
            cycleTime = MAX_CYCLE_TIME;
        }
    }else{
        cycleTime = 0;
    }
    *g_cycleCount = cycleTime;

    // read last soh(saved before last poweroff)
    double soh_saved[CELL_NUMS];
    double soh_first_powerup[CELL_NUMS];
    memset(soh_first_powerup, 0xff, sizeof(soh_first_powerup));
    double soh_abnormal_flag[CELL_NUMS];
    ret = read_saved_soh(soh_saved);
    if(ret == 0)
    {
        if(memcmp(soh_saved, ))
        
    }else{
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            soh_abnormal_flag[i] = true;
        }
        
    }

    double soh = 100 - 20 * (*g_cycleCount/1000.0/REFERENCE_CYCLE_TIME);
    for(size_t i = 0; i < CELL_NUMS; i++)
    {
        soh_saved[i] = soh;
        soh_abnormal_flag[i] = 0;

    }




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


void soh_save()
{

}



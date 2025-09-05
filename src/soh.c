#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "sox.h"
#include "sox_private.h"
#include "port.h"
#include "sox_config.h"
#include "soh.h"


static float s_lastSOC[CELL_NUMS];
static float s_lastGrpSOC;
int8_t  soh_init()
{
    // read last cycle time(saved before last poweroff)
    uint32_t cycleTime;
    int8_t ret = read_saved_cycle(&cycleTime);
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
    g_cycleCount = (float)cycleTime;

    // read last soh(saved before last poweroff)
    float soh_saved[CELL_NUMS];
    float soh_first_powerup[CELL_NUMS];
    memset(soh_first_powerup, 0xff, sizeof(soh_first_powerup));
    bool soh_abnormal_flag[CELL_NUMS];
	memset(soh_abnormal_flag, false, sizeof(soh_abnormal_flag));
    ret = read_saved_soh(soh_saved);
    if(ret == 0)
    {
        if(memcmp(soh_saved, soh_first_powerup, sizeof soh_first_powerup) == 0){
            for (size_t i = 0; i < CELL_NUMS; i++)
            {
                soh_abnormal_flag[i] = true;
            }
        }else{
            for (size_t i = 0; i < CELL_NUMS; i++)
            {
                if(soh_saved[i] > 100 || soh_saved[i] < 0){
                    soh_abnormal_flag[i] = true;
                }
            }
        }
        
    }else{
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            soh_abnormal_flag[i] = true;
        }
        
    }

    float soh = 100 - 20 * (g_cycleCount/1000.0f/REFERENCE_CYCLE_TIME);
    for(size_t i = 0; i < CELL_NUMS; i++)
    {
        if(soh_abnormal_flag[i] == true){
            g_celSOH[i] = soh;
        }else{
            g_celSOH[i] = soh_saved[i];
        }
    }




    memcpy(s_lastSOC, g_celSOC, sizeof(s_lastSOC));
    s_lastGrpSOC = g_grpSOC;


	soh_save(FORCE_SAVE);
    return 0;
}





int8_t soh_task()
{

    // bug: lost some cycle when charge change to discharge or discharge to charge
    if(g_grpSOC <= s_lastGrpSOC-1){
        g_cycleCount += (s_lastGrpSOC-g_grpSOC)/200.0f;
        s_lastGrpSOC  = g_grpSOC;
    }else if(g_grpSOC >= s_lastGrpSOC+1){
        g_cycleCount += (g_grpSOC-s_lastGrpSOC)/200.0f;
        s_lastGrpSOC  = g_grpSOC;
        
    }

    float sumSOH = 0;
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        if (g_celSOC[i] <= s_lastSOC[i]-1 || g_celSOC[i] >= s_lastSOC[i]+1)
        {
            float delta = fabsf(g_celSOC[i] - s_lastSOC[i]);
            delta = delta*0.01f;
            if(g_celTmp[i] <= 250)
            {
                float subk = 20.0f/5000;
                g_celSOH[i] -= delta*0.5f*subk;
            }else if(g_celTmp[i] < 450)
            {
                float subk = 20.0f/(5000-((g_celTmp[i] - 250)/200.0f)*3000);
                g_celSOH[i] -= delta*0.5f*subk;
            }else{
                float subk = 20.0f/2000;
                g_celSOH[i] -= delta*0.5f*subk;
            }
            s_lastSOC[i] = g_celSOC[i];
        }
        sumSOH += g_celSOH[i];
    }
    
    g_grpSOH = sumSOH/CELL_NUMS;

    return 0;
}


void soh_save(bool force)
{
    if(!force){
        static float  last_cycleCount = 0;
        bool save_flag = false;
        static uint32_t save_time = 0;
        if(last_cycleCount - g_cycleCount > 1)
        {
            save_flag = true;
            last_cycleCount = g_cycleCount;
        }
        if(timebase_get_time_s() - save_time > 60*60*24*7)
        {
            if(save_flag){
                write_saved_cycle((uint32_t)g_cycleCount);
                write_saved_soh(g_celSOH);
            }
            save_time = timebase_get_time_s();
        }
    }else{
            write_saved_cycle((uint32_t)g_cycleCount);
            write_saved_soh(g_celSOH);
    }

}



#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include "port.h"
#include "sox_config.h"
#include "soc.h"
#include "sox_private.h"

#define UNUSED(x) (void)(x)


extern uint32_t g_excel_second;

int   g_port_init_soc;                                                          // input from command line argument
float s_init_accChgAH = 0;
float s_init_accChgWH = 0;
float s_init_accDsgAH = 0;
float s_init_accDsgWH = 0;
uint32_t timebase_get_time_s(void)
{
    return g_excel_second;
}


int8_t read_saved_soc(float *soc)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        soc[i] = g_port_init_soc;                                               // input from command line argument
    }

    return 0;
}
int8_t write_saved_soc(float *soc)
{
    FILE* save_soc = fopen("save_soc.txt", "w");
    fprintf(save_soc, "%d -> ", g_excel_second);
    for(int i = 0; i < CELL_NUMS; i++)
    {
        fprintf(save_soc, "%f ", *soc);
    }
    fprintf(save_soc, "\n");
    fclose(save_soc);

    return 0;
}



int8_t read_saved_soc_group(float *grpsoc)
{
    *grpsoc = g_port_init_soc;
    return 0;
}
int8_t write_saved_soc_group(float grpsoc)
{
    FILE* save_soc_grp = fopen("save_soc_grp.txt", "w");
    fprintf(save_soc_grp, "%d -> ", g_excel_second);

    fprintf(save_soc_grp, "%f", grpsoc);

    fprintf(save_soc_grp, "\n");
    fclose(save_soc_grp);
    return 0;
}

int8_t read_saved_soh(float *soh)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        soh[i] = 100;
    }
    
    return 0;
}
int8_t write_saved_soh(float *soh)
{   
    UNUSED(soh);
    return 0;
}

int8_t read_saved_cycle(uint32_t *cycleTime)
{
    *cycleTime = 0;
    return 0;
}
int8_t write_saved_cycle(uint32_t cycleTime)
{
    UNUSED(cycleTime);
    return 0;
}

int8_t read_saved_soe(float *totalChgWh, float *totalDsgWh, float *totalChgAh, float *totalDsgAh)
{
    *totalChgWh = s_init_accChgWH;
    *totalDsgWh = s_init_accDsgWH;
    *totalChgAh = s_init_accChgAH;
    *totalDsgAh = s_init_accDsgAH;

    return 0;
}

int8_t write_saved_soe(float totalChgWh, float totalDsgWh, float totalChgAh, float totalDsgAh)
{
    UNUSED(totalChgWh);
    UNUSED(totalDsgWh);
    UNUSED(totalChgAh);
    UNUSED(totalDsgAh);

    return 0;
}



__attribute__((weak)) uint32_t get_cpu_tick()
{

    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (uint32_t)(tv.tv_sec*1000000+tv.tv_usec);

}




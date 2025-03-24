#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include "port.h"
#include "sox_config.h"
#include "soc.h"
#include "sox_private.h"


#define SOC0                                100
#define SOC0_ER2                            25

#if FULL_STD_CLIB


#else

void __aeabi_assert (const char *expr, const char *file, int line) {
    while(1);
}


#endif


extern uint32_t excel_second;
uint32_t timebase_get_time_s(void)
{


    return excel_second;


}


int8_t read_saved_soc(float *soc)
{

    return 0;
}
int8_t write_saved_soc(float *soc)
{
    FILE* save_soc = fopen("save_soc.txt", "a+");
    fprintf(save_soc, "%d -> ", excel_second);
    for(int i = 0; i < CELL_NUMS; i++)
    {
        fprintf(save_soc, "%f ", *soc);
    }
    fprintf(save_soc, "\n");
    fclose(save_soc);

    return 0;
}



int8_t read_saved_soc_group(uint16_t *grpsoc)
{

    return 0;
}
int8_t write_saved_soc_group(uint16_t grpsoc)
{
    FILE* save_soc_grp = fopen("save_soc_grp.txt", "a+");
    fprintf(save_soc_grp, "%d -> ", excel_second);

    fprintf(save_soc_grp, "%f", grpsoc);

    fprintf(save_soc_grp, "\n");
    fclose(save_soc_grp);
    return 0;
}

int8_t read_saved_soh(float *soh)
{

    return 0;
}
int8_t write_saved_soh(float *soh)
{
    return 0;



}

int8_t read_saved_cycle(uint32_t *cycleTime)
{

    return 0;
}
int8_t write_saved_cycle(uint32_t cycleTime)
{
    return 0;



}

int8_t read_saved_soe(float *totalChgWh, float *totalDsgWh)
{

    return 0;
}

int8_t write_saved_soe(float totalChgWh, float totalDsgWh)
{

    return 0;
}




void port_sox_input(void)
{




}

void port_sox_output(void)
{




}


void port_soc_input(void)
{




}


void port_soc_output(void)
{



    
}



void port_soe_input(void)
{




}


void port_soe_output(void)
{



    
}

void port_soh_input(void)
{




}


void port_soh_output(void)
{



    
}



void port_soc_init(void)
{
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        g_socInfo[i].soc = SOC0;
        g_socInfo[i].socEr2 = SOC0_ER2;
        g_celSOC[i] = round(SOC0);

    }
    *g_grpSOC = round(SOC0);
}
void port_soh_init(void)
{
    for(size_t i = 0; i < CELL_NUMS; i++)
    {
        g_celSOH[i] = 100;
    }

}
void port_soe_init(void)
{


}
void port_sop_init(void)
{



}








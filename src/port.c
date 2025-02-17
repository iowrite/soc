#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include "port.h"
#include "sox_config.h"

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



int8_t read_saved_soc_group(float *grpsoc)
{

    return 0;
}
int8_t write_saved_soc_group(float grpsoc)
{
    FILE* save_soc_grp = fopen("save_soc_grp.txt", "a+");
    fprintf(save_soc_grp, "%d -> ", excel_second);

    fprintf(save_soc_grp, "%f", grpsoc);

    fprintf(save_soc_grp, "\n");
    fclose(save_soc_grp);
    return 0;
}

int8_t read_saved_soh(double *soh)
{

    return 0;
}
int8_t write_saved_soh(double *soh)
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









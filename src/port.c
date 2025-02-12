#include <stdint.h>
#include <time.h>
#include "port.h"


uint32_t timebase_get_time_s(void)
{

    time_t seconds;
    seconds = time(NULL);

    return (uint32_t)seconds;


}


int8_t read_saved_soc(float *soc)
{

    return 0;
}
int8_t write_saved_soc(float *soc)
{
    return 0;



}



int8_t read_saved_soc_group(float *grpsoc)
{

    return 0;
}
int8_t write_saved_soc_group(float grpsoc)
{
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







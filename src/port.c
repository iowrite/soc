#include <stdint.h>
#include "port.h"
#include "soc.h"
#include "sox_private.h"
#include "common.h"


__attribute__((weak)) uint32_t timebase_get_time_s(void)
{
    return 0;
}
__attribute__((weak)) int8_t read_saved_soc(float *soc)
{
    UNUSED(soc);
    return 0;
}
__attribute__((weak)) int8_t write_saved_soc(float *soc)
{
    UNUSED(soc);
    return 0;
}
__attribute__((weak)) int8_t read_saved_soc_group(float *grpsoc)
{
    UNUSED(grpsoc);
    return 0;
}
__attribute__((weak)) int8_t write_saved_soc_group(float grpsoc)
{
    UNUSED(grpsoc);
    return 0;
}

__attribute__((weak)) int8_t read_saved_soh(float *soh)
{
    UNUSED(soh);
    return 0;
}
__attribute__((weak)) int8_t write_saved_soh(float *soh)
{
    UNUSED(soh);
    return 0;
}

__attribute__((weak)) int8_t read_saved_cycle(uint32_t *cycleTime)
{
    UNUSED(cycleTime);
    return 0;
}
__attribute__((weak)) int8_t write_saved_cycle(uint32_t cycleTime)
{
    UNUSED(cycleTime);
    return 0;
}

__attribute__((weak)) int8_t read_saved_soe(float *totalChgWh, float *totalDsgWh, float *totalChgAh, float *totalDsgAh)
{
    UNUSED(totalChgWh);
    UNUSED(totalDsgWh);
    UNUSED(totalChgAh);
    UNUSED(totalDsgAh);
    return 0;
}

__attribute__((weak)) int8_t write_saved_soe(float totalChgWh, float totalDsgWh, float totalChgAh, float totalDsgAh)
{
    UNUSED(totalChgWh);
    UNUSED(totalDsgWh);
    UNUSED(totalChgAh);
    UNUSED(totalDsgAh);
    return 0;
}

__attribute__((weak)) uint32_t get_cpu_tick(void)
{

    return 0;
}











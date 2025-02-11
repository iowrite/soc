#include <stdint.h>
#include <time.h>
#include "timebase.h"


uint32_t timebase_get_time_s(void)
{

    time_t seconds;
    seconds = time(NULL);

    return (uint32_t)seconds;


}
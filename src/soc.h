#include <stdint.h>
#include <stdbool.h>
#include "sox_config.h"
#ifndef _SOX_SOC_H
#define _SOX_SOC_H

struct SOC_Info
{
    bool pureAH_lock;
    uint8_t cell_vol_buff_idx;
    uint16_t cell_vol_buffer[CELL_VOL_BUFFER_LEN];
    uint32_t record_time;
    float soc_smooth;
    float soc;
    float socEr2;
    const uint16_t *last_curve;
    uint16_t swith_curve_time;

};

extern struct SOC_Info g_socInfo[CELL_NUMS];

void soc_init();
void soc_task(bool full, bool empty);
void soc_save(bool force);

#endif


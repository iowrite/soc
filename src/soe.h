#include <stdint.h>
#ifndef _SOX_SOE_H
#define _SOX_SOE_H


int8_t soe_init(void);

int8_t soe_task(void);

void soe_save(bool force);

#endif
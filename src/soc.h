#include <stdint.h>
#include <stdbool.h>

#ifndef _SOC_H
#define _SOC_H



void soc_init();
void soc_task(bool full, bool empty);
void soc_save();

#endif


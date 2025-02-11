#include <stdint.h>
#ifndef AEKF_PORT_H
#define AEKF_PORT_H

/* 
* @brief get a timestamp in seconds
*/
uint32_t timebase_get_time_s(void);

/* 
* @brief read saved soc (last soc before shutdown)
* @param soc pointer to float array of soc
* @return  0: valid
*         -1: invalid
*/
int8_t read_saved_soc(float *soc);

/* 
* @brief write lastest soc (per 1% changed)
* @param soc pointer to float array of soc
* @return  0: valid
*         -1: invalid
*/
int8_t write_saved_soc(float *soc);


#endif
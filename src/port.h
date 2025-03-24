#include <stdint.h>
#ifndef _SOX_PORT_H
#define _SOX_PORT_H

#define FULL_STD_CLIB       1

/**
 * @brief get a timestamp in seconds
 */
uint32_t timebase_get_time_s(void);

/**
 * @brief read saved soc (last soc before shutdown)
 * @param soc pointer to float array of soc
 * @return  0: valid
 *         -1: invalid
 */
int8_t read_saved_soc(float *soc);


/**
 * @brief read saved soc (last soc before shutdown)
 * @param soc pointer to uint16_t group soc
 * @return  0: valid
 *         -1: invalid
 */
int8_t read_saved_soc_group(uint16_t *grpsoc);


/** 
 * @brief write lastest soc (per 1% changed)
 * @param soc pointer to float array of soc
 * @return  0: valid
 *         -1: invalid
 */
int8_t write_saved_soc(float *soc);


/** 
 * @brief write lastest soc (per 1% changed)
 * @param soc uint16_t group soc
 * @return  0: valid
 *         -1: invalid
 */
int8_t write_saved_soc_group(uint16_t grpsoc);


/** 
 * @brief read saved soh (last soh before shutdown)
 * @param soh pointer to float array of soh
 * @return  0: valid
 *         -1: invalid
 */
int8_t read_saved_soh(float *soh);

/** 
 * @brief write lastest soh (per 1% changed)
 * @param soh pointer to float array of soh
 * @return  0: valid
 *         -1: invalid
 */
int8_t write_saved_soh(float *soh);

/** 
 * @brief read saved cycle (last cycle before shutdown)
 * @param cycleTime pointer to uint32_t cycleTime
 * @return  0: valid
 *         -1: invalid
 */
int8_t read_saved_cycle(uint32_t *cycleTime);
/** 
 * @brief write saved cycle (last cycle before shutdown)
 * @param cycleTime uint32_t cycleTime
 * @return  0: valid
 *         -1: invalid
 */
int8_t write_saved_cycle(uint32_t cycleTime);


/** 
 * @brief read saved soe (last soe before shutdown)
 * @param totalChgWh pointer to float accmulated charge energy
 * @param totalDsgWh pointer to float accmulated discharge energy
 * @return  0: valid
 *         -1: invalid
 */

int8_t read_saved_soe(float *totalChgWh, float *totalDsgWh);
/** 
 * @brief write saved soe (last soe before shutdown)
 * @param totalChgWh float accmulated charge energy
 * @param totalDsgWh float accmulated discharge energy
 * @return  0: valid
 *         -1: invalid
 */
int8_t write_saved_soe(float totalChgWh, float totalDsgWh);


void port_sox_input(void);
void port_sox_output(void);



void port_soc_input(void);
void port_soc_output(void);

void port_soh_input(void);
void port_soh_output(void);

void port_soe_input(void);
void port_soe_output(void);

void port_sop_input(void);
void port_sop_output(void);


void port_soc_init(void);
void port_soh_init(void);
void port_soe_init(void);
void port_sop_init(void);



#endif
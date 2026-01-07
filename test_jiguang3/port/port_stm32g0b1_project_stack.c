#include <stdint.h>
#include "port.h"
#include "soc.h"
#include "sox_private.h"
#include "common.h"

#include "sox_config.h"
#include "drv_eeprom.h"
#include "User_DataStore.h"
#include "drv_tim.h"

#define SOC_STORE_ADDR				EEPROM_SOX_PARA_START_ADDR		            // page start
#define SOC_STORE_LEN				80

#define GRP_SOC_STORE_ADDR			(SOC_STORE_ADDR+SOC_STORE_LEN)
#define GRP_SOC_STORE_LEN			48

#define SOH_STORE_ADDR				(GRP_SOC_STORE_ADDR+GRP_SOC_STORE_LEN)      // page start
#define SOH_SOC_STORE_LEN			80

#define CYCLE_STORE_ADDR			(SOH_STORE_ADDR+SOH_SOC_STORE_LEN)
#define CYCLE_SOC_STORE_LEN			48

#define SOE_STORE_ADDR				(CYCLE_STORE_ADDR+CYCLE_SOC_STORE_LEN)      // page start
#define SOE_SOC_STORE_LEN			16










uint32_t timebase_get_time_s(void)
{
    return Tim_Loop.Cnt_s;
}
int8_t read_saved_soc(float *soc)
{
	int ret = Api_EepromFunc.EEPROM_Read(SOC_STORE_ADDR, (uint8_t *)soc, sizeof(float)*CELL_NUMS);
    return ret;
}
int8_t write_saved_soc(float *soc)
{
	int ret = Api_EepromFunc.EEPROM_Write(SOC_STORE_ADDR, (uint8_t *)soc, sizeof(float)*CELL_NUMS);
    return ret;
}
int8_t read_saved_soc_group(float *grpsoc)
{
	int ret =  Api_EepromFunc.EEPROM_Read(GRP_SOC_STORE_ADDR, (uint8_t *)grpsoc, sizeof(float));
    return ret;
}
int8_t write_saved_soc_group(float grpsoc)
{
	int ret =  Api_EepromFunc.EEPROM_Write(GRP_SOC_STORE_ADDR, (uint8_t *)&grpsoc, sizeof(float));
    return ret;
}

int8_t read_saved_soh(float *soh)
{
	int ret = Api_EepromFunc.EEPROM_Read(SOH_STORE_ADDR, (uint8_t *)soh, sizeof(float)*CELL_NUMS);
    return ret;
}
int8_t write_saved_soh(float *soh)
{
	int ret = Api_EepromFunc.EEPROM_Write(SOH_STORE_ADDR, (uint8_t *)soh, sizeof(float)*CELL_NUMS);
    return ret;
}

int8_t read_saved_cycle(uint32_t *cycleTime)
{
	int ret =  Api_EepromFunc.EEPROM_Read(CYCLE_STORE_ADDR, (uint8_t *)cycleTime, sizeof(uint32_t));
    return ret;
}
int8_t write_saved_cycle(uint32_t cycleTime)
{
    int ret =  Api_EepromFunc.EEPROM_Write(CYCLE_STORE_ADDR, (uint8_t *)&cycleTime, sizeof(uint32_t));
    return ret;
}

int8_t read_saved_soe(float *totalChgWh, float *totalDsgWh, float *totalChgAh, float *totalDsgAh)
{
    UNUSED(totalChgAh);
    UNUSED(totalDsgAh);
    
	float buff[2] = {};
    int ret =  Api_EepromFunc.EEPROM_Read(SOE_STORE_ADDR, (uint8_t *)buff, sizeof(buff));
	*totalChgWh = buff[0];
	*totalDsgWh = buff[1];
    return ret;
}

int8_t write_saved_soe(float totalChgWh, float totalDsgWh, float totalChgAh, float totalDsgAh)
{
    UNUSED(totalChgAh);
    UNUSED(totalDsgAh);
    
	float buff[2] = {totalChgWh, totalDsgWh};
    int ret =  Api_EepromFunc.EEPROM_Write(SOE_STORE_ADDR, (uint8_t *)buff, sizeof(buff));
    return ret;
}









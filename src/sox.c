#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "sox_private.h"
#include "sox_config.h"
#include "soc.h"
#include "soh.h"
#include "soe.h"
#include "sox.h"
#include "port.h"


/*******************************************************************************
 * global varible instant(sox module internal use only)
 ******************************************************************************/
float        g_cur;                         // A          
float        g_grpVol;                      // v      
uint16_t     g_celVol[CELL_NUMS];;          // mv                   pointer may be better(small size but can't promise origin data be changed by other module)
int16_t      g_celTmp[CELL_NUMS];;          // dregree              pointer may be better(small size but can't promise origin data be changed by other module)
float        g_celSOC[CELL_NUMS];           // %         
float        g_grpSOC;                      // %       
float        g_celSOH[CELL_NUMS];           // %        
float        g_grpSOH;                      // %            
float        g_cycleCount;                  // times
float        g_accChgWH;                    // WH          
float        g_accDsgWH;                    // WH           
float        g_sigChgWH;                    // WH           
float        g_sigDsgWH;                    // WH     
float        g_accChgAH;                    // AH          
float        g_accDsgAH;                    // AH      
uint16_t    *g_chg_stop_vol;                // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)     
uint16_t    *g_dsg_stop_vol;                // mv                   pass in by init func, use pointer to sync parameter changed by user(user operation)   
bool         g_empty;            
bool         g_full;                




int8_t sox_init(struct SOX_Init_Attr *attr)
{
    g_chg_stop_vol  = attr->chg_stop_vol;
    g_dsg_stop_vol  = attr->dsg_stop_vol;
    soc_init();
    soh_init();
    soe_init();

    return 0;
} 



static void sox_input(const struct SOX_Input *input)
{
    g_cur = input->cur;
    memcpy(g_celVol, input->vol, sizeof g_celVol);
    memcpy(g_celTmp, input->tmp, sizeof g_celTmp);
    g_grpVol = input->grpVol;
    g_empty  = input->empty;
    g_full   = input->full;

}

int8_t sox_task(const struct SOX_Input *input)
{
    // input
    sox_input(input);

    // calculate
    soc_task(g_full, g_empty);
    soh_task();
    soe_task();


    // Periodic storage to prevent power outages
    soc_save(AUTO_SAVE);                 // cell soc and group soc
    soh_save(AUTO_SAVE);                 // cell soh , group soh and cycle count
    soe_save(AUTO_SAVE);                 // accumulate charge and discharge energy
	
    // save once, if the current is less than CUR_WINDOW_A, it means that the system is in standby state
	static uint32_t standby_save = 0;
	if(fabsf(g_cur)< CUR_WINDOW_A)
	{
		standby_save++;
		if(standby_save == 1){
			soc_save(FORCE_SAVE);                 
			soh_save(FORCE_SAVE);            
			soe_save(FORCE_SAVE); 
		}
	}else{
		standby_save = 0;
	}
     
    return 0;
}


int8_t sox_manual_set_soc(float soc)
{
    for(int i = 0; i < CELL_NUMS; i++){
        g_socInfo[i].soc = soc;
        g_celSOC[i] = soc;
		if(g_socInfo[i].soc_smooth)
		{
			g_socInfo[i].soc_smooth = soc;
		}
    }
    g_grpSOC = (uint16_t)soc;

    soc_save(true);
    return 0;
}

int8_t sox_manual_set_soh(float soh, uint32_t cycleCount)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        g_celSOH[i] = soh;
    }
    g_grpSOH = soh;
    g_cycleCount = cycleCount;

    soh_save(true);
    return 0;
}



int8_t sox_manual_set_acc_chg_dsg(float accChgWH, float accDsgWH)
{
    g_accChgWH = accChgWH;
    g_accDsgWH = accDsgWH;

    soe_save(true);
    return 0;
}



/*******************************************************************************
 * output functions
 *******************************************************************************/
 
float get_cell_soc(uint16_t cellIndex)
{
    return g_celSOC[cellIndex];
}
float get_group_soc(void)
{
    return g_grpSOC;
}
float get_cell_soh(uint16_t cellIndex)
{
    return g_celSOH[cellIndex];
}
float get_group_soh(void)
{
    return g_grpSOH;
}
float get_cycle_count(void)
{
    return g_cycleCount;
}
float get_sig_chg_wh(void)
{
    return g_sigChgWH;
}
float get_sig_dsg_wh(void)
{
    return g_sigDsgWH;
}
float get_acc_chg_wh(void)
{
    return g_accChgWH;
}
float get_acc_dsg_wh(void)
{
    return g_accDsgWH;
}
float get_acc_chg_ah(void)
{
    return g_accChgAH;
}
float get_acc_dsg_ah(void)
{
    return g_accDsgAH;
}

int8_t get_cell_soc_ary(float *socAry)                                          // fix len: CELL_NUMS
{
    memcpy(socAry, g_celSOC, sizeof(g_celSOC));
    return 0;
}

int8_t get_cell_soh_ary(float *sohAry)                                          // fix len: CELL_NUMS
{
    memcpy(sohAry, g_celSOH, sizeof(g_celSOH));
    return 0;
}
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include "sox.h"
#include "sox_config.h"
#include "sox_private.h"
#include "port.h"
#include "soh.h"
#include "common.h"
#include "curve.h"
#include "debug.h"


static float s_lastSOC[CELL_NUMS];
static float s_lastGrpSOC;
int8_t  soh_init(void)
{
    // read last cycle time(saved before last poweroff)
    uint32_t cycleTime;
    int8_t ret = read_saved_cycle(&cycleTime);
    if(ret == 0){
        if(cycleTime == 0xffffffff){    // first powerup(empty eeprom or flash)
            cycleTime = 0;
        }else if(cycleTime>MAX_CYCLE_TIME)
        {
            cycleTime = MAX_CYCLE_TIME;
        }
    }else{
        cycleTime = 0;
    }
    g_cycleCount = (float)cycleTime;

    // read last soh(saved before last poweroff)
    float soh_saved[CELL_NUMS];
    float soh_first_powerup[CELL_NUMS];
    memset(soh_first_powerup, 0xff, sizeof(soh_first_powerup));
    bool soh_abnormal_flag[CELL_NUMS];
	memset(soh_abnormal_flag, false, sizeof(soh_abnormal_flag));
    ret = read_saved_soh(soh_saved);
    if(ret == 0)
    {
        if(memcmp(soh_saved, soh_first_powerup, sizeof soh_first_powerup) == 0){
            for (size_t i = 0; i < CELL_NUMS; i++)
            {
                soh_abnormal_flag[i] = true;
            }
        }else{
            for (size_t i = 0; i < CELL_NUMS; i++)
            {
                if(soh_saved[i] > 100 || soh_saved[i] < 0){
                    soh_abnormal_flag[i] = true;
                }
            }
        }
        
    }else{
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            soh_abnormal_flag[i] = true;
        }
        
    }

    float soh = 100 - 20 * (g_cycleCount/1000.0f/REFERENCE_CYCLE_TIME);
    for(size_t i = 0; i < CELL_NUMS; i++)
    {
        if(soh_abnormal_flag[i] == true){
            g_celSOH[i] = soh;
        }else{
            g_celSOH[i] = soh_saved[i];
        }
    }

    float sumSOH = 0;
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        sumSOH += g_celSOH[i];
    }
    g_grpSOH = sumSOH / CELL_NUMS;



    memcpy(s_lastSOC, g_celSOC, sizeof(s_lastSOC));
    s_lastGrpSOC = g_grpSOC;


	soh_save(FORCE_SAVE);
    return 0;
}



enum Battery_State{
    Battery_State_standby,
    Battery_State_charging,
    Battery_State_discharging,
};

struct Battery_State_Record{
    enum Battery_State type;            // process of charging or discharging
    float group_SOC;
    int16_t min_cell_temp;
    uint32_t tick;
}; 

static struct Battery_State_Record s_recored_start;
static struct Battery_State_Record s_recored_end;

void battey_state_record(void)
{
    static enum Battery_State state = Battery_State_standby;
    int16_t sorted_cell_temp[CELL_NUMS];
    bubbleSort_ascend_int16(g_celTmp, sorted_cell_temp, ARRAY_SIZE(g_celTmp));
    int16_t min_cell_temp = sorted_cell_temp[0]; 
    switch (state) {
        case Battery_State_standby:
            s_recored_end.type = Battery_State_standby;
            s_recored_end.group_SOC = g_grpSOC;
            s_recored_end.tick = timebase_get_time_s();
            s_recored_end.min_cell_temp = min_cell_temp;
            if(g_cur > CUR_WINDOW_A){
                state = Battery_State_charging;
                s_recored_start.type = Battery_State_charging;
                s_recored_start.group_SOC = g_grpSOC;
                s_recored_start.tick = timebase_get_time_s();
                s_recored_start.min_cell_temp = min_cell_temp;
            }else if(g_cur < -CUR_WINDOW_A){
                state = Battery_State_discharging;
                s_recored_start.type = Battery_State_discharging;
                s_recored_start.group_SOC = g_grpSOC;
                s_recored_start.tick = timebase_get_time_s();
                s_recored_start.min_cell_temp = min_cell_temp;
            }
            break;
        case Battery_State_charging:
            s_recored_end.type = Battery_State_charging;
            s_recored_end.group_SOC = g_grpSOC;
            s_recored_end.tick = timebase_get_time_s();
            s_recored_end.min_cell_temp = min_cell_temp;
            if(fabsf(g_cur) < CUR_WINDOW_A){
                state = Battery_State_standby;
                s_recored_start.type = Battery_State_standby;
                s_recored_start.group_SOC = g_grpSOC;
                s_recored_start.tick = timebase_get_time_s();
                s_recored_start.min_cell_temp = min_cell_temp;
            }else if(g_cur < -CUR_WINDOW_A){
                state = Battery_State_discharging;
                s_recored_start.type = Battery_State_discharging;
                s_recored_start.group_SOC = g_grpSOC;
                s_recored_start.tick = timebase_get_time_s();
                s_recored_start.min_cell_temp = min_cell_temp;
            }
            break;
        case Battery_State_discharging:
            s_recored_end.type = Battery_State_discharging;
            s_recored_end.group_SOC = g_grpSOC;
            s_recored_end.tick = timebase_get_time_s();
            s_recored_end.min_cell_temp = min_cell_temp;
            if(fabsf(g_cur) < CUR_WINDOW_A){
                state = Battery_State_standby;
                s_recored_start.type = Battery_State_standby;
                s_recored_start.group_SOC = g_grpSOC;
                s_recored_start.tick = timebase_get_time_s();
                s_recored_start.min_cell_temp = min_cell_temp;
            }else if(g_cur > CUR_WINDOW_A){
                state = Battery_State_charging;
                s_recored_start.type = Battery_State_charging;
                s_recored_start.group_SOC = g_grpSOC;
                s_recored_start.tick = timebase_get_time_s();
                s_recored_start.min_cell_temp = min_cell_temp;
            }
            break;
        default:
            break;
    }
}


#if SOX_CFG_H_BAT_LAW
/******************************************************************************
 * remain power capalility
 ******************************************************************************/
static void remain_power_capability_task(void)
{
    
    uint32_t cycles = (uint32_t)g_cycleCount;
    
    uint32_t table_lookup_i = cycles/100;
    
    if(table_lookup_i >= ARRAY_SIZE(remain_power_capability_table))
    {
        table_lookup_i = ARRAY_SIZE(remain_power_capability_table) - 1;
    }
    
    g_remain_power_capability = remain_power_capability_table[table_lookup_i];

}



/******************************************************************************
 * remain energy convert effciency 
 ******************************************************************************/
#if SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N == 1
static int month_acc_chg_energy[30] = {};
static int month_acc_dsg_energy[30] = {};
static int month_acc_chg_energy_idx_rear,   month_acc_dsg_energy_idx_rear;
static bool month_acc_chg_energy_ary_full,   month_acc_dsg_energy_ary_full;
void remain_energy_conv_eff_task_m1(void)
{
    static int just_now; 
    static bool first_call = true;
    if(first_call){
        first_call = false;
        just_now = get_time_of_day_from_use();
    }
    int now;
    now = get_time_of_day_from_use();
    if(now != just_now){
        int item_num = now-just_now;
        for(int i = 0; i < item_num; i++){
            if(i != item_num-1){
                month_acc_chg_energy[month_acc_chg_energy_idx_rear] = -1;
                month_acc_dsg_energy[month_acc_dsg_energy_idx_rear] = -1;
            }else{
                month_acc_chg_energy[month_acc_chg_energy_idx_rear] = g_accChgWH;
                month_acc_dsg_energy[month_acc_dsg_energy_idx_rear] = g_accDsgWH;
            }
            month_acc_chg_energy_idx_rear++;
            month_acc_dsg_energy_idx_rear++;
            if(month_acc_chg_energy_idx_rear >= sizeof month_acc_chg_energy / sizeof *month_acc_chg_energy)
            {
                month_acc_chg_energy_idx_rear = 0;
                month_acc_chg_energy_ary_full = 1;
            }
            if(month_acc_dsg_energy_idx_rear >= sizeof month_acc_dsg_energy / sizeof *month_acc_dsg_energy)
            {
                month_acc_dsg_energy_idx_rear = 0;
                month_acc_dsg_energy_ary_full = 1;
            }
        }
        just_now = now;        
    }
        
}

#define remain_energy_conv_eff_active_point     5
int get_remain_energy_conv_eff_m1(void)
{
    float month_diff_chg_energy = 0;
    float month_diff_dsg_energy = 0;
    bool acc_chg_valid = false, acc_dsg_valid = false;
    int acc_chg_tail = 0;
    if(month_acc_chg_energy_ary_full)
    {
        acc_chg_tail = month_acc_chg_energy_idx_rear + 1;
        
        if(acc_chg_tail >= sizeof month_acc_chg_energy / sizeof *month_acc_chg_energy){
            acc_chg_tail = 0;
        }
    }
    if(month_acc_chg_energy_ary_full){             // full
        month_diff_chg_energy = month_acc_chg_energy[month_acc_chg_energy_idx_rear] - month_acc_chg_energy[acc_chg_tail];
        if(month_diff_chg_energy > remain_energy_conv_eff_active_point)
        {
            acc_chg_valid = true;
        }
    }
       
    int acc_dsg_tail = 0;    
    if(month_acc_dsg_energy_ary_full)
    {
        acc_dsg_tail = month_acc_dsg_energy_idx_rear + 1;
        
        if(acc_dsg_tail >= sizeof month_acc_dsg_energy / sizeof *month_acc_dsg_energy){
            acc_dsg_tail = 0;
        }
    }
    if(month_acc_dsg_energy_ary_full){                  // full
            
        month_diff_dsg_energy = month_acc_dsg_energy[month_acc_dsg_energy_idx_rear] - month_acc_dsg_energy[acc_dsg_tail];
        if(month_diff_dsg_energy > remain_energy_conv_eff_active_point)
        {
            acc_dsg_valid = true;
        }
    }
        
    if(acc_chg_valid && acc_dsg_valid){
        float conv_eff = month_diff_dsg_energy / month_diff_chg_energy;
        return conv_eff;
    }
    
    return 0;
}
#endif


#if SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N == 2
void remain_energy_conv_eff_task_m2(void)
{

    uint32_t cycles = (uint32_t)g_cycleCount;
    
    uint32_t table_lookup_i = cycles/100;
    
    if(table_lookup_i >= ARRAY_SIZE(energy_convert_efficiency_table))
    {
        table_lookup_i = ARRAY_SIZE(energy_convert_efficiency_table) - 1;
    }
    
    g_remain_energy_conv_eff = energy_convert_efficiency_table[table_lookup_i];


}

int get_remain_energy_conv_eff_m2(void)
{
    return g_remain_energy_conv_eff;
}




#endif




/******************************************************************************
 * evolution of self discharge
 ******************************************************************************/

void evolu_of_selfdsg_task(void)
{
    int days = get_time_of_day_from_use();
    
    int table_lookup_i = days/180;
    
    table_lookup_i--;
    if(table_lookup_i < 0)
    {
        table_lookup_i = 0;
    } 

    if(table_lookup_i >= (int)ARRAY_SIZE(remain_self_dsg_table))
    {
        table_lookup_i = ARRAY_SIZE(remain_self_dsg_table) - 1;
    }
    g_evolu_of_selfdsg = remain_self_dsg_table[table_lookup_i];
}





/******************************************************************************
 * deep charge times
 ******************************************************************************/
#define DEEP_DSG_VOL                2500
#define DEEP_DSG_VOL_RESTORE        2550

void deep_dsg_task(void)
{
    uint16_t sorted_vol[CELL_NUMS];
    bubbleSort_ascend_uint16(g_celVol, sorted_vol, ARRAY_SIZE(g_celVol));
    uint16_t minvol = sorted_vol[0];
    static bool stay_in_under_vol = false;
    if(!stay_in_under_vol && minvol < DEEP_DSG_VOL)
    {
        g_deep_dsg_cnt++;
        stay_in_under_vol = true;
    }
    if(minvol > DEEP_DSG_VOL_RESTORE){
        stay_in_under_vol = false;
    }

}







/******************************************************************************
 * deep charge times
 ******************************************************************************/
#define DEEP_CHG_VOL            3750
#define DEEP_CHG_VOL_RESTORE    3700

void deep_chg_task(void)
{
    uint16_t sorted_vol[CELL_NUMS];
    bubbleSort_ascend_uint16(g_celVol, sorted_vol, ARRAY_SIZE(g_celVol));
    uint16_t maxvol = g_celVol[CELL_NUMS-1];
    static bool stay_in_over_vol = false;
    if(!stay_in_over_vol && maxvol > DEEP_CHG_VOL)
    {
        g_deep_chg_cnt++;
        stay_in_over_vol = true;
    }
    if(maxvol < DEEP_CHG_VOL_RESTORE){
        stay_in_over_vol = false;
    }
}





/******************************************************************************
 * time of extreme temperature
 ******************************************************************************/
#define EXTREME_TEMP_HIGH       1550
#define EXTREME_TEMP_LOW        800

void timeof_extreme_temp_task(void)
{
    int16_t sorted_cell_temp[CELL_NUMS];
    bubbleSort_ascend_int16(g_celTmp, sorted_cell_temp, ARRAY_SIZE(g_celTmp));
    int min_temp = sorted_cell_temp[0];
    int max_temp =sorted_cell_temp[CELL_NUMS-1];
    static uint64_t lasttime_min;
    static bool counting = false;
    if(min_temp < EXTREME_TEMP_LOW || max_temp > EXTREME_TEMP_HIGH)
    {
        if(!counting){
            lasttime_min = get_time_min();
        }
        counting = true;
        uint64_t now = get_time_min();
        if(now - lasttime_min >= 1){
            g_extreme_time_cnt++;
            lasttime_min = now;
        }
    }
    if(min_temp >= EXTREME_TEMP_LOW && max_temp <= EXTREME_TEMP_HIGH){
        counting = false;
    }
   
}




/******************************************************************************
 * charge time of extreme temperature
 ******************************************************************************/

#define EXTREME_CHG_TEMP_HIGH       1550
#define EXTREME_CHG_TEMP_LOW        1000
#define CUR_WINDOW                  2

void timeof_extreme_chg_task(void)
{
    int16_t sorted_cell_temp[CELL_NUMS];
    bubbleSort_ascend_int16(g_celTmp, sorted_cell_temp, ARRAY_SIZE(g_celTmp));
    int min_chg_temp = sorted_cell_temp[0];
    int max_chg_temp = sorted_cell_temp[CELL_NUMS-1];
    int cur = (int)g_cur;
    static uint64_t lasttime_min;
    static bool counting = false;
    if((min_chg_temp < EXTREME_CHG_TEMP_LOW || max_chg_temp > EXTREME_CHG_TEMP_HIGH) && cur > CUR_WINDOW)
    {
        
        if(!counting){
            lasttime_min = get_time_min();
        }
        counting = true;
        uint64_t now = get_time_min();
        if(now - lasttime_min >= 1){
            g_extreme_chg_time_cnt++;
            lasttime_min = now;
        }
    }
    if(min_chg_temp > EXTREME_CHG_TEMP_LOW && max_chg_temp < EXTREME_CHG_TEMP_HIGH && cur <= -CUR_WINDOW)
    {
        counting = false;
    }


}


int8_t bat_law_cal_init(void)
{
    int8_t ret = 0;
    ret += read_remain_power_capability(&g_remain_power_capability);
    ret += read_remain_energy_conv_eff(&g_remain_energy_conv_eff);
    ret += read_evolu_of_selfdsg(&g_evolu_of_selfdsg);
    ret += read_num_of_deep_dsg(&g_deep_chg_cnt);
    ret += read_num_of_deep_chg(&g_deep_dsg_cnt);
    ret += read_time_of_extreme_temp(&g_extreme_chg_time_cnt);
    ret += read_time_of_extreme_chg(&g_extreme_time_cnt);

    return ret;
}

int8_t write_bat_store(void)
{
    int8_t ret = 0;
    ret += write_remain_power_capability(g_remain_power_capability);
    ret += write_remain_energy_conv_eff(g_remain_energy_conv_eff);
    ret += write_evolu_of_selfdsg(g_evolu_of_selfdsg);
    ret += write_num_of_deep_dsg(g_deep_chg_cnt);
    ret += write_num_of_deep_chg(g_deep_dsg_cnt);
    ret += write_time_of_extreme_temp(g_extreme_chg_time_cnt);
    ret += write_time_of_extreme_chg(g_extreme_time_cnt);

    return ret;
}


static void bat_law_cal_task(void)
{
    remain_power_capability_task();
#if SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N == 1
    remain_energy_conv_eff_task_m1();
#elif SOX_CFG_H_REMAIN_ENERGY_CONV_EFF_M_N == 2
    remain_energy_conv_eff_task_m2();
#endif
    evolu_of_selfdsg_task();
    deep_dsg_task();
    deep_chg_task();
    timeof_extreme_temp_task();
    timeof_extreme_chg_task();
    

    static int just_now;
    int now = get_time_of_day_from_use();
    if(now != just_now){
        just_now = now;
        write_bat_store();
    }

}


#endif

int8_t soh_task(void)
{

    // bug: lost some cycle when charge change to discharge or discharge to charge
    if(g_grpSOC <= s_lastGrpSOC-1){
        g_cycleCount += (s_lastGrpSOC-g_grpSOC)/200.0f;
        s_lastGrpSOC  = g_grpSOC;
    }else if(g_grpSOC >= s_lastGrpSOC+1){
        g_cycleCount += (g_grpSOC-s_lastGrpSOC)/200.0f;
        s_lastGrpSOC  = g_grpSOC;
        
    }

    float sumSOH = 0;
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        if (g_celSOC[i] <= s_lastSOC[i]-1 || g_celSOC[i] >= s_lastSOC[i]+1)
        {
            float delta = fabsf(g_celSOC[i] - s_lastSOC[i]);
            delta = delta*0.01f;
            if(g_celTmp[i] <= SOH_LOW_TEMP)
            {
                float subk = (100-SOH_CYCLE_L1_PERCENT)/SOH_LOW_TEMP_CYCLE_L1;
                g_celSOH[i] -= delta*0.5f*subk;
            }else if(g_celTmp[i] < SOH_HIGH_TEMP)
            {
                float subk = (100-SOH_CYCLE_L1_PERCENT)/(SOH_LOW_TEMP_CYCLE_L1-((g_celTmp[i] - SOH_LOW_TEMP)/200.0f)*(SOH_LOW_TEMP_CYCLE_L1-SOH_HIGH_TEMP_CYCLE_L1));
                g_celSOH[i] -= delta*0.5f*subk;
            }else{
                float subk = (100-SOH_CYCLE_L1_PERCENT)/SOH_HIGH_TEMP_CYCLE_L1;
                g_celSOH[i] -= delta*0.5f*subk;
            }
            s_lastSOC[i] = g_celSOC[i];
        }
        sumSOH += g_celSOH[i];
    }
    
    battey_state_record();

    float soh_calibrate = 0;
    if(g_soh_calibrate_tigger == SOH_CALIBRATION_TIGGERED_BY_CHARGING)
    {   
        g_soh_calibrate_tigger = 0;
        if(s_recored_start.type == Battery_State_charging 
            && s_recored_start.group_SOC < SOH_PASSIVE_GRP_SOC_CHG_START
            && s_recored_start.min_cell_temp > SOH_PASSIVE_CALIBRATE_TEMP_LIMIT
            && s_recored_end.min_cell_temp > SOH_PASSIVE_CALIBRATE_TEMP_LIMIT)
        {
            DEBUG_LOG("charge soh_calibratem , jump from %f to 100 \n", g_group_soc_before_jump);
            float jump_diff = 100-g_group_soc_before_jump;
            soh_calibrate = jump_diff/10;
            if(soh_calibrate > 1){
                soh_calibrate = 1;
            }
        }
    }else if(g_soh_calibrate_tigger == SOH_CALIBRATION_TIGGERED_BY_DISCHARGING)
    {
        g_soh_calibrate_tigger = 0;
        if(s_recored_start.type == Battery_State_discharging 
            && s_recored_start.group_SOC > SOH_PASSIVE_GRP_SOC_DSG_START
            && s_recored_start.min_cell_temp > SOH_PASSIVE_CALIBRATE_TEMP_LIMIT
            && s_recored_end.min_cell_temp > SOH_PASSIVE_CALIBRATE_TEMP_LIMIT)
        {
            DEBUG_LOG("discharge soh_calibratem , jump from %f to 0 \n", g_group_soc_before_jump);
            float jump_diff = g_group_soc_before_jump;
            soh_calibrate = jump_diff/5;
            if(soh_calibrate > 1){
                soh_calibrate = 1;
            }
        }
    }

    if(soh_calibrate != 0)
    {
        for(int i = 0; i < CELL_NUMS; i++)
        {
            g_celSOH[i] -= soh_calibrate;
        }
    }

    g_grpSOH = sumSOH/CELL_NUMS-soh_calibrate;

#if SOX_CFG_H_BAT_LAW
    bat_law_cal_task();
#endif

    return 0;
}


void soh_save(bool force)
{
    if(!force){
        static float  last_cycleCount = 0, last_soh = 100;
        bool save_flag = false;
        static uint32_t save_time = 0;
        if(g_cycleCount - last_cycleCount > 1)
        {
            save_flag = true;
            last_cycleCount = g_cycleCount;
        }
        if(last_soh - g_grpSOH >= 1)
        {
            save_flag = true;
            last_soh = g_grpSOH;
        }
        if(timebase_get_time_s() - save_time > SOX_CFG_H_SAVE_CHECK_TIME)
        {
            if(save_flag){
#if SOX_DEBUG_SOH_SAVE
                DEBUG_LOG("soh save, group soh is %f\n", g_grpSOH);
#endif
                write_saved_cycle((uint32_t)g_cycleCount);
                write_saved_soh(g_celSOH);
            }
            save_time = timebase_get_time_s();
        }
    }else{
#if SOX_DEBUG_SOH_SAVE
            DEBUG_LOG("soh save, group soh is %f\n", g_grpSOH);
#endif
            write_saved_cycle((uint32_t)g_cycleCount);
            write_saved_soh(g_celSOH);
    }

}



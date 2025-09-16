#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "sox_config.h"
#if FULL_STD_CLIB
    #include <assert.h>
#endif 
    #include <stdlib.h>
#if PORT_ARM_AC_6
    #include <cmsis_armclang.h>
#endif
#include "soc.h"
#include "curve.h"
#include "sox_private.h"
#include "sox.h"
#include "port.h"
#include "common.h"
#include "debug.h"
#include "soh.h"

#define EKF_W(diffAH, cap, cur)                 ((50.0f/(DIFF_T_SEC*1000) + CUR_SAMPLE_ERR_A/cur + CAP_ERR_AH/cap+SOH_ERR_PERCENT/100.0f) *  EKF_Q_K * diffAH)     
#define EKF_Q(diffAH, cap, cur)                 (EKF_W(diffAH, cap, cur)*EKF_W(diffAH, cap, cur))                 
#define EKF_R_1                                 (VOL_SAMPLE_ERR_MV_1*VOL_SAMPLE_ERR_MV_1)
#define EKF_R_2                                 (VOL_SAMPLE_ERR_MV_2*VOL_SAMPLE_ERR_MV_2)
#define EKF_R_3                                 (VOL_SAMPLE_ERR_MV_3*VOL_SAMPLE_ERR_MV_3)

#define SOC0_ER2_SAVED                      100             // 10% error
#define SOC0_ER2_LOOKUP_TABLE               900             // 30% error

#define CHARGING(cur)       ((cur) > 0)
#define DISCHARGING(cur)    ((cur) < 0)




struct SOC_Info g_socInfo[CELL_NUMS];

static uint16_t get_cap(float cur, int16_t tempra)
{
    int t_table[] = {-150, -50, 50, 150, 250, 350, 450, 550};
    int tidx = 0;
    tidx = (tempra+200)/100;
    float tk = fabsf((tempra % 50)/50.0f);
    int tidx_low, tidx_high = 0;
    if(tidx < 0){
        tidx = 0;
        tidx_low = tidx;
        tidx_high = tidx + 1;
    }else if(tidx >= TEMP_POINT_NUM)
    {
        tidx = TEMP_POINT_NUM-1;
        tidx_low = tidx;
        tidx_high = TEMP_POINT_NUM-1;
    }else{
        tidx_low = tidx;
        tidx_high = tidx+1;
    }


    float mk;
    if(tempra > 0){
        mk = SOC_TEMPRA_WARM_CAP_OFFSET_1;
    }else{
        mk = SOC_TEMPRA_WARM_CAP_OFFSET_2;
    }

    if(cur > 0)
    {
        float c = cur/100*10;
        int cidx = (int)roundf(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        if(s_cap_list_chg[tidx][cidx] == 0){
#if FULL_STD_CLIB
            assert(0);
#endif
        }
        if(tempra < t_table[tidx])
        {
            return s_cap_list_chg[tidx][cidx];
        }else{
           return (uint16_t)(s_cap_list_chg[tidx_low][cidx] + (s_cap_list_chg[tidx_high][cidx]-s_cap_list_chg[tidx_low][cidx])*tk*mk); 
        }
    }else if(cur < 0)
    {
        float c = cur/100*10;
        int cidx = -(int)roundf(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        if(s_cap_list_dsg[tidx][cidx] == 0){
#if FULL_STD_CLIB
            assert(0);
#endif
        }
        if(tempra < t_table[tidx])
        {
            return s_cap_list_dsg[tidx][cidx];
        }else{
            return (uint16_t)(s_cap_list_dsg[tidx_low][cidx] + (s_cap_list_dsg[tidx_high][cidx]-s_cap_list_dsg[tidx_low][cidx])*tk*mk);
        }
    }

    return 0;
}




static const uint16_t * get_v(const uint16_t *chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], const uint16_t *dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], float cur, int16_t tempra)
{

    int tidx = 0;
    tidx = (tempra+200)/100;
    if(tidx < 0){
        tidx = 0;
    }else if(tidx >= TEMP_POINT_NUM)
    {
        tidx = TEMP_POINT_NUM-1;
    }

    if(cur > 0)
    {
        float c = cur/100*10;
        int cidx = (int)roundf(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
#if FULL_STD_CLIB
        assert(chg_curve[tidx][cidx] != NULL);
#endif
        return chg_curve[tidx][cidx];

    }else if(cur < 0)
    {
        float c = cur/100*10;
        int cidx = -(int)roundf(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
#if FULL_STD_CLIB
        assert(dsg_curve[tidx][cidx] != NULL);
#endif
        return dsg_curve[tidx][cidx];
    }
    return NULL;
}


static const int16_t * get_k(const int16_t *chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], const int16_t *dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], float cur, int16_t tempra)
{
    int tidx = 0;
    tidx = (tempra+200)/100;
    if(tidx < 0){
        tidx = 0;
    }else if(tidx >= TEMP_POINT_NUM)
    {
        tidx = TEMP_POINT_NUM-1;
    }

    if(cur > 0)
    {
        float c = cur/100*10;
        int cidx = (int)round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
#if FULL_STD_CLIB
        assert(chg_curve[tidx][cidx] != NULL);
#endif
        return chg_curve[tidx][cidx];

    }else if(cur < 0)
    {
        float c = cur/100*10;
        int cidx = -(int)round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
#if FULL_STD_CLIB
        assert(dsg_curve[tidx][cidx] != NULL);
#endif
        return dsg_curve[tidx][cidx];
    }
    return NULL;
}



static const uint16_t * get_curve_v(float cur, int16_t tempra)
{
    return get_v(s_chg_curve, s_dsg_curve, cur, tempra);
}

static const int16_t * get_curve_k(float cur, int16_t tempra)
{
    return get_k(s_chg_curve_k, s_dsg_curve_k, cur, tempra);
}




enum GroupState
{
    GROUP_STATE_standby,
    GROUP_STATE_charging,
    GROUP_STATE_discharging,
    GROUP_STATE_transfer,
};
static enum GroupState g_group_state;




/**
 * @brief detect system state: standy, charge, discharge, transformation
 * @note 1. standby to charge/discharge, go to transform state(in the first 30 minutes)
 */
static enum GroupState last_group_state = GROUP_STATE_standby;
static uint32_t standby_hold_time = 0;
static uint32_t trans_dsg_acc_time = 0;
static uint32_t trans_chg_acc_time = 0;
enum GroupState check_current_group_state(float cur)
{

    switch (g_group_state)
    {
    case GROUP_STATE_standby:
        trans_dsg_acc_time++;
        trans_chg_acc_time++;

        if  ( cur > CUR_WINDOW_A && (last_group_state == GROUP_STATE_standby || last_group_state == GROUP_STATE_charging))          
        {
            g_group_state = GROUP_STATE_charging;
        }else if(cur < -CUR_WINDOW_A && (last_group_state == GROUP_STATE_standby || last_group_state == GROUP_STATE_discharging)) 
        {
            g_group_state = GROUP_STATE_discharging;                      
        }
        if(timebase_get_time_s()-standby_hold_time < STANDBY_HOLD_TIME){
            if(cur>CUR_WINDOW_A && last_group_state == GROUP_STATE_discharging){
                g_group_state = GROUP_STATE_transfer;
                trans_dsg_acc_time = 0;
                trans_chg_acc_time = 0;
            }else if(cur<-CUR_WINDOW_A && last_group_state == GROUP_STATE_charging){
                g_group_state = GROUP_STATE_transfer;
                trans_dsg_acc_time = 0;
                trans_chg_acc_time = 0;
            }
        }
        break;
    case GROUP_STATE_transfer:
        if(fabsf(cur) < CUR_WINDOW_A){                               // reset state logic
            g_group_state = GROUP_STATE_standby;
            standby_hold_time = timebase_get_time_s();
        }else if(cur > CUR_WINDOW_A){
            last_group_state = GROUP_STATE_charging;
            trans_dsg_acc_time = 0;
            trans_chg_acc_time++;
            if(trans_chg_acc_time > GROUP_STATE_CHANGE_TIME){
                g_group_state = GROUP_STATE_charging;
                last_group_state = GROUP_STATE_transfer;
                trans_dsg_acc_time = 0;
                trans_chg_acc_time = 0;
            }
        }else if(cur < -CUR_WINDOW_A){
            last_group_state = GROUP_STATE_discharging;
            trans_chg_acc_time = 0;
            trans_dsg_acc_time++;
            if(trans_dsg_acc_time > GROUP_STATE_CHANGE_TIME){
                g_group_state = GROUP_STATE_discharging;
                last_group_state = GROUP_STATE_transfer;
                trans_dsg_acc_time = 0;
                trans_chg_acc_time = 0;
            }
        }
        break;
    case GROUP_STATE_charging:
        if(fabsf(cur)<CUR_WINDOW_A){
            g_group_state = GROUP_STATE_standby;
            standby_hold_time = timebase_get_time_s();
            last_group_state = GROUP_STATE_charging;
            // check_current_group_state(cur);
        }else if(cur<-CUR_WINDOW_A){
            g_group_state = GROUP_STATE_transfer;
            last_group_state = GROUP_STATE_charging;
        }
        break;
    case GROUP_STATE_discharging:
        if(fabsf(cur)<CUR_WINDOW_A){
            g_group_state = GROUP_STATE_standby;
            standby_hold_time = timebase_get_time_s();
            last_group_state = GROUP_STATE_discharging;
            // check_current_group_state(cur);
        }else if(cur>CUR_WINDOW_A){
            g_group_state = GROUP_STATE_transfer;
            last_group_state = GROUP_STATE_discharging;
        }
        break;
    default:
        break;
    }
    return g_group_state;
}


void mysoc_pureAH(struct SOC_Info *SOCinfo, float cur, uint16_t vol, int16_t tempra, float soh)
{
    UNUSED(vol);
    const uint16_t cap = get_cap(cur, tempra);

    const float capf = cap/10.0f*(soh/100);

    float diffAH = 100*DIFF_T_SEC/3600.0f*cur/capf;
    float SOCcal = SOCinfo->soc + diffAH;
    float SOCer2Cal = SOCinfo->socEr2 + EKF_Q(diffAH,capf, cur);



    if(SOCcal < 0)
    {
        SOCcal = 0;
    }else if(SOCcal > 100)
    {
        SOCcal = 100;
    }

    SOCinfo->soc = SOCcal;
    SOCinfo->socEr2 = SOCer2Cal;

}





void mysoc_smooth(struct SOC_Info *SOCinfo, float cur, uint16_t vol, int16_t tempra, float soh)
{
#if SOX_DEBUG
    static int callcount = 0;
    callcount++;
    UNUSED(callcount);      // avoid warning in debug mode
#endif 
    UNUSED(tempra);
    UNUSED(soh);

    float newest_vol, oldest_vol;
    if(timebase_get_time_s()-SOCinfo->record_time > CELL_VOL_BUFFER_SAMPLE_TIME_S){
            SOCinfo->record_time = timebase_get_time_s();
            SOCinfo->cell_vol_buffer[SOCinfo->cell_vol_buff_idx] = vol;
            SOCinfo->cell_vol_buff_idx++;
            if(SOCinfo->cell_vol_buff_idx >= CELL_VOL_BUFFER_LEN){
                SOCinfo->cell_vol_buff_idx = 0;
            }
    }
    if(SOCinfo->cell_vol_buff_idx == 1){
        newest_vol = SOCinfo->cell_vol_buffer[0];
        oldest_vol = SOCinfo->cell_vol_buffer[CELL_VOL_BUFFER_LEN-1];
    }else if(SOCinfo->cell_vol_buff_idx == 0){
        newest_vol = SOCinfo->cell_vol_buffer[CELL_VOL_BUFFER_LEN-1];
        oldest_vol = SOCinfo->cell_vol_buffer[CELL_VOL_BUFFER_LEN-2];
    }else{
        newest_vol = SOCinfo->cell_vol_buffer[SOCinfo->cell_vol_buff_idx-1];
        oldest_vol = SOCinfo->cell_vol_buffer[SOCinfo->cell_vol_buff_idx-2];
    }
    


    if((cur > 0 && vol > SOC_SMOOTH_START_VOL_CHG &&  SOCinfo->soc > 80) || (cur < 0 && vol < SOC_SMOOTH_START_VOL_DSG && SOCinfo->soc < 30))
    {
        if(SOCinfo->soc_smooth == 0){
            SOCinfo->soc_smooth = SOCinfo->soc;                  
        }

        if(cur>0  && ((int)newest_vol-(int)oldest_vol)){
            int smooth_full_estimate_s = (int)((*g_chg_stop_vol-vol)/(newest_vol-oldest_vol)*(CELL_VOL_BUFFER_LEN-1)*CELL_VOL_BUFFER_SAMPLE_TIME_S);
            UNUSED(smooth_full_estimate_s);                                                         // charge not use force smooth mothod(cell voltage curve of all "sop compatible situation" is  monotonically increasing, so temporary disable)
            
        }else if(cur<0 && ((int)oldest_vol-(int)newest_vol)){
			float buff_diff = oldest_vol-newest_vol;
			if(buff_diff < 1)			// current change, cell voltage restore briefly
			{
				buff_diff = 1;
			}

            if(vol>*g_dsg_stop_vol){
                int smooth_empty_estimate_s = (int)((vol-*g_dsg_stop_vol)/buff_diff*(CELL_VOL_BUFFER_LEN-1)*CELL_VOL_BUFFER_SAMPLE_TIME_S);
                float smoothDiff = SOCinfo->soc_smooth/(float)smooth_empty_estimate_s*(float)(2+(int)(20*(vol-*g_dsg_stop_vol)/(SOC_SMOOTH_START_VOL_DSG-*g_dsg_stop_vol)));
                SOCinfo->soc_smooth -= smoothDiff;

            }


        }
    }

    if(SOCinfo->soc_smooth < 0)
    {
        SOCinfo->soc_smooth = 0.1f;             // not equal to 0(zero mean smooth not enable)
    }else if(SOCinfo->soc_smooth > 100)
    {
        SOCinfo->soc_smooth = 100;
    }

}







static uint32_t getEKF_R(float H_soc, uint16_t vol, const uint16_t *curve, const int16_t *curveK, uint16_t switch_curve_time)
{
    int index = 0;
    for(int i = 0; i < SOC_POINT_NUM; i++)
    {
        if(vol > curve[i])
        {
            index++;
        }else{
            break;
        }
    }
    float K;                    // curve slope
    if(index == 0)
    {
        K = curveK[0];
    }else if(index == SOC_POINT_NUM)
    {
        K = curveK[SOC_POINT_NUM-1];
    }else{
        K = (curveK[index-1] + (float)(curveK[index]-curveK[index-1])*(vol-curve[index-1])/(curve[index]-curve[index-1]))/10.0f;
    }

    if(K < H_soc)
    {
        K = H_soc;
    }

    uint32_t H_R;
    if(K<VOL_SAMPLE_ERR_MV_1_H)
    {
        H_R = VOL_SAMPLE_ERR_MV_1*VOL_SAMPLE_ERR_MV_1;
    }else if(K>VOL_SAMPLE_ERR_MV_4_H)
    {
        H_R = VOL_SAMPLE_ERR_MV_4*VOL_SAMPLE_ERR_MV_4;
    }else{
        int t = abs(VOL_SAMPLE_ERR_MV_2-VOL_SAMPLE_ERR_MV_3);
        H_R = (uint32_t)(VOL_SAMPLE_ERR_MV_3+(K-VOL_SAMPLE_ERR_MV_1_H)/(VOL_SAMPLE_ERR_MV_4_H-VOL_SAMPLE_ERR_MV_1_H)*(float)t)
             *(uint32_t)(VOL_SAMPLE_ERR_MV_3+(K-VOL_SAMPLE_ERR_MV_1_H)/(VOL_SAMPLE_ERR_MV_4_H-VOL_SAMPLE_ERR_MV_1_H)*(float)t);
    }
    if(switch_curve_time){
        return H_R+25*switch_curve_time/600;                                    

    }else{
        return H_R;
    }
    
  
}



void mysocEKF(struct SOC_Info *SOCinfo, float cur, uint16_t vol, int16_t tempra, float soh)
{
#if SOX_DEBUG
    static int callCount = 0;
    callCount++;
    static float pureAHSUM = 0;
    UNUSED(callCount);      // avoid warning in debug mode
    UNUSED(pureAHSUM);      // avoid warning in debug mode
#endif   

    const uint16_t *curve = get_curve_v(cur, tempra);
    const int16_t *curveK = get_curve_k(cur, tempra);
    const uint16_t cap = get_cap(cur, tempra);

    if(SOCinfo->last_curve != curve)
    {
        SOCinfo->last_curve = curve;
        SOCinfo->swith_curve_time = 600;
    }
    if(SOCinfo->swith_curve_time != 0)
    {
        SOCinfo->swith_curve_time--;
    }

    const float capf = cap/10.0f*(soh/100);

    float diffAH = DIFF_T_SEC/3600.0f*cur/capf*100;
    float SOCcal = SOCinfo->soc + diffAH;
    float Q = EKF_Q(diffAH, capf, cur);
    float SOCer2Cal = SOCinfo->socEr2 + Q;
#if SOX_DEBUG
    pureAHSUM += diffAH;
#endif
    float H = 0, Hprev = 0, Hnext = 0;
    float estVol = 0, estVolPrev = 0, estVolNext = 0;
    if(SOCcal < 0)
    {
        SOCcal = 0;

        estVol = curve[0];
        H = (float)curveK[0]/10;
    }else if(SOCcal > 100)
    {
        SOCcal = 100;

        estVol = curve[SOC_POINT_NUM-1];
        H = (float)curveK[SOC_POINT_NUM-1]/10;
    }else{
        float unused_interger_part;
        if(SOCcal < 5)
        {
            float sock = modff(SOCcal, &unused_interger_part);
            estVolPrev = curve[(int)SOCcal];
            estVolNext = curve[(int)SOCcal+1];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }else if(SOCcal < 95)
        {
            float sock = fmodf(SOCcal, 5)/5;
            estVolPrev = curve[(int)(SOCcal/5)+4];
            estVolNext = curve[(int)(SOCcal/5)+1+4];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }    
        else{
            float sock = modff(SOCcal, &unused_interger_part);
            estVolPrev = curve[23+(int)SOCcal-95];
            estVolNext = curve[23+(int)SOCcal-95+1];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }

        if(SOCcal < 5)
        {
            float Hk = modff(SOCcal, &unused_interger_part);
            Hprev = (float)curveK[((int)SOCcal)]/10;
            Hnext = (float)curveK[(int)SOCcal+1]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }else if(SOCcal < 95)
        {
            float Hk = fmodf(SOCcal,5)/5;
            Hprev = (float)curveK[((int)(SOCcal/5)+4)]/10;
            Hnext = (float)curveK[(int)(SOCcal/5)+1+4]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }    
        else{
            float Hk = modff(SOCcal, &unused_interger_part);
            Hprev = (float)curveK[23+(int)SOCcal-95]/10;
            Hnext = (float)curveK[23+(int)SOCcal-95+1]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }
    }
    float K = 0;
    uint32_t ekfR = getEKF_R(H, vol, curve, curveK, SOCinfo->swith_curve_time);

    K = SOCer2Cal*H/(H*SOCer2Cal*H+(float)ekfR);
    
    float res = SOCcal+K*((float)vol-estVol);
    float resEr2 = (1-K*H)*SOCer2Cal;
#if SOX_DEBUG
    // float SOCerRes = sqrt(resEr2);
#endif 

    if(res < 0)
    {
        res = 0;
    }else if(res > 100)
    {
        res = 100;
    }

    if(cur > 0){
        if(res < SOCinfo->soc)
        {
            float smooth_k = 10/SOCinfo->soc;
            if(smooth_k > 1){
                smooth_k = 1;
            }
            res = SOCinfo->soc + smooth_k*diffAH;
        }
    }else if(cur < 0)
    {
        if(res > SOCinfo->soc)
        {
            float smooth_k = 10/(100-SOCinfo->soc);
            if(smooth_k > 1){
                smooth_k = 1;
            }
            res = SOCinfo->soc + smooth_k*diffAH;
        }
    }

    if(res < 0)
    {
        res = 0;
    }else if(res > 100)
    {
        res = 100;
    }

    SOCinfo->soc = res;
    SOCinfo->socEr2 = resEr2;
    
}






void mysoc(struct SOC_Info *SOCinfo, float cur, uint16_t vol, int16_t tempra, float soh)
{
    if(fabsf(cur) > CUR_WINDOW_A)
    {
        if((tempra<0 && tempra > -200) || (tempra < PURE_AH_LOCK_TEMP_THRESHOLD && cur < -PURE_AH_LOCK_CUR_THRESHOLD)){
            SOCinfo->pureAH_lock = true;
        }
        if(SOCinfo->pureAH_lock)
        {
            mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);
            mysoc_smooth(SOCinfo, cur, vol, tempra, soh);
           DEBUG_LOG("pureAH lock\n");
        }else{
            if(g_group_state == GROUP_STATE_transfer){
                mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);               // Ampere-hour Integration only
            }else{
                mysocEKF(SOCinfo, cur, vol, tempra, soh);                   // Ampere-hour Integration + EKF
            }
        }
    }else if (cur != 0){
        mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);
    }


}



#define GRP_Q_MAX               25
#define GRP_Q_Min               1
#define GRP_Q_1                   0.01f
#define GRP_Q_2                   0.1f
#define GRP_Q_3                   0.0002f

#define  E_HIGH_MIN       0.1f
#define  E_HIGH_MAX       5
#define  E_LOW_MIN        0.1f
#define  E_LOW_MAX        5


#define  R_HIGH_MIN       (E_HIGH_MIN*E_HIGH_MIN)
#define  R_HIGH_MAX       (E_HIGH_MAX*E_HIGH_MAX)
#define  R_LOW_MIN        (E_LOW_MIN*E_LOW_MIN)
#define  R_LOW_MAX        (E_LOW_MAX*E_LOW_MAX)
#define  R_AVG_MIN        (E_AEG_MIN*E_AEG_MIN)
#define  R_AVG_MAX        (E_AEG_MIN*E_AEG_MIN)

static void gropuSOC(void)
{
#if SOX_DEBUG
    static int callCount = 0;
    callCount++;
    UNUSED(callCount);      // avoid warning in debug mode
#endif 
    float unsortedSOC[CELL_NUMS];
    bool pureAH_lock = false;
    for(int i = 0; i < CELL_NUMS; i++)
    {
        if(g_socInfo[i].soc_smooth > 0)         // smooth enabled
        {
            if(CHARGING(g_cur) &&  g_socInfo[i].soc_smooth>g_socInfo[i].soc){
                unsortedSOC[i] = g_socInfo[i].soc_smooth;

            }else if(DISCHARGING(g_cur) &&  g_socInfo[i].soc_smooth<g_socInfo[i].soc){
                unsortedSOC[i] = g_socInfo[i].soc_smooth;
            }else{
                unsortedSOC[i] = g_socInfo[i].soc;
            }
        }else{
            unsortedSOC[i] = g_socInfo[i].soc;
        }
        if(g_socInfo[i].pureAH_lock)
        {
            pureAH_lock = true;
        }
        
    }
    float sortedSOC[CELL_NUMS];
    bubbleSort_ascend_float(unsortedSOC, sortedSOC, CELL_NUMS);
    float maxSOC = sortedSOC[CELL_NUMS-1];
    float minSOC = sortedSOC[0];
    float avgSOC = 0;
    for(int i=0; i<CELL_NUMS; i++)
    {
        avgSOC += sortedSOC[i];
    }
    avgSOC = avgSOC/CELL_NUMS;



    float max_soc_change_R_offset = fabsf(maxSOC - avgSOC);
    float min_soc_change_R_offset = fabsf(minSOC - avgSOC);
    float max_soc_change_R_offset2 = max_soc_change_R_offset*max_soc_change_R_offset;
    float min_soc_change_R_offset2 = min_soc_change_R_offset*min_soc_change_R_offset;


    if(max_soc_change_R_offset2 > 9)
    {
        max_soc_change_R_offset2 = 9; 
        if(maxSOC > 97)
        {
            max_soc_change_R_offset2 = 9*(100-maxSOC)/3; 
        }
    }
    if(min_soc_change_R_offset2 > 9)
    {
        min_soc_change_R_offset = 9; 
        if(minSOC < 5)
        {
            min_soc_change_R_offset2 = 9*minSOC/5;
        }
    }
    if(pureAH_lock)
    {
        max_soc_change_R_offset2 = 0;
        min_soc_change_R_offset2 = 0;
    }

    float maxSOC_R = R_HIGH_MIN + (1-g_grpSOC/100) * (R_HIGH_MAX- R_HIGH_MIN) + max_soc_change_R_offset2;
    float minSOC_R = R_LOW_MIN + g_grpSOC/100 * (R_LOW_MAX- R_LOW_MIN) + min_soc_change_R_offset2;


    if(g_grpSOC > 50)
    {
        if(minSOC_R > 25)
        {
            minSOC_R = 25;
        }
        if(maxSOC_R > minSOC_R){
            maxSOC_R = minSOC_R -1;
        }
    }else if(g_grpSOC < 50)
    {
        if(maxSOC_R > 25)
        {
            maxSOC_R = 25;
        }
        if(minSOC_R > maxSOC_R)
        {
            minSOC_R = maxSOC_R -1;
        }
    }else{
        if(minSOC_R > 25)
        { 
            minSOC_R = 25;
        }
        if(maxSOC_R > 25)
        {
            maxSOC_R = 25;
        }
        if(g_cur > 0)
        {
            if(maxSOC_R > minSOC_R){
                maxSOC_R = minSOC_R -1;
            }
        }
        if(g_cur < 0)
        {
            if(minSOC_R > maxSOC_R)
            {
                minSOC_R = maxSOC_R -1;
            }
        }
    }

    static float cal_grp_soc;
    static bool grp_soc_init = false;
    if(!grp_soc_init){
        grp_soc_init = true;
        cal_grp_soc = g_grpSOC;
    }
    

    static float grp_soc_p = 0.1f;
    float cal_grp_soc_p;
    if(CHARGING(g_cur) && g_grpSOC > 95)
    {
        cal_grp_soc_p=grp_soc_p + GRP_Q_1;
    }else if(CHARGING(g_cur) && g_grpSOC < 10)
    {
        cal_grp_soc_p=grp_soc_p + GRP_Q_2;
    }else{
        float grp_soc_q_k = fabsf(g_cur)/10;
        if(grp_soc_q_k < 0.5f)
        {
            grp_soc_q_k = 0.5f;
        }
        cal_grp_soc_p=grp_soc_p + GRP_Q_3*grp_soc_q_k;
    }


    float H[2][1] = {
        {1}, 
        {1}
    };
    float H_t[1][2] = {
        {1, 1}
    };
    float Z[2][1] = {
        {minSOC}, 
        {maxSOC}
    };
    float R[2][2] = {    
        {minSOC_R, 0        },
        {0       , maxSOC_R },
    };
    float S[2][2] = {0};
    float H_P[2][1] = {
        {cal_grp_soc_p}, 
        {cal_grp_soc_p}
    };
    matrix_multiply((float *)H_P, (float *)H_t, (float *)S, 2, 1, 2);
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<2; j++)
        {
            S[i][j] = S[i][j] + R[i][j];
        }

    }

    float S_i[2][2] = {0};
    inverse_matrix_2x2(S, S_i);

    float P_H_t[1][2] = {
        {cal_grp_soc_p, cal_grp_soc_p}
    };

    float K[1][2] = {0};
    matrix_multiply((float *)P_H_t, (float *)S_i, (float *)K, 1, 2, 2);


    float Z_H[2][1] = {0};
    float H_x[2][1] = {
        {cal_grp_soc}, 
        {cal_grp_soc}
    };
    for(int i=0; i<2; i++)
    {
        Z_H[i][0] = Z[i][0] - H_x[i][0];
    }

    float x_k = 0;
    matrix_multiply((float *)K, (float *)Z_H, &x_k, 1, 2, 1);

    cal_grp_soc = cal_grp_soc + x_k;

    float K_H = 0;
    matrix_multiply((float *)K, (float *)H, &K_H, 1, 2, 1);
    grp_soc_p = (1-K_H)*cal_grp_soc_p;


    // when soc increasing, soc upper limit 99%, when soc decreasing, soc lower limit 1%
    if(cal_grp_soc > g_grpSOC)
    {
        if(cal_grp_soc > 99)
        {
            cal_grp_soc = 99;
        }
    }else if (cal_grp_soc < g_grpSOC)
    {
        if(cal_grp_soc < 1){
            cal_grp_soc = 1;
        }
    }

    // avoid soc abnormal reverse jump
    if(CHARGING(g_cur) && cal_grp_soc < g_grpSOC)
    {
        return;
    }
    if(DISCHARGING(g_cur) && cal_grp_soc > g_grpSOC)
    {
        return; 
    }

        
#if SOC_FAKE_SMOOTH_ENABLE
    static uint32_t smooth_count = 0;
    float smooth_soc = g_grpSOC;
    if(fabsf(cal_grp_soc - g_grpSOC)>=2)
    {        
        if(timebase_get_time_s()-smooth_count > (uint32_t)2)
        {
            smooth_count = timebase_get_time_s();
            if(cal_grp_soc > g_grpSOC)
            {
                smooth_soc++;
            }else{
                smooth_soc--;
            }
            if(smooth_soc > 99)
            {
                smooth_soc = 99;
            }else if(smooth_soc < 1){
                smooth_soc = 1;
            }
            g_grpSOC = smooth_soc;

        }
    }else{
        g_grpSOC = cal_grp_soc;
    }
#else
    g_grpSOC = cal_grp_soc;
#endif

}

/**
 * @brief  convert voltage to soc by lookup table
 * @return soc
 */
static float vol2soc(uint16_t vol, int16_t tempra)
{
    UNUSED(tempra);                                     // not used now, but may be used in future
    float soc = 0;
    
    for(int i = 0; i < OCV_POINT_NUM; i++)
    {
        if(vol > temp_ocv[4][i])
        {
            soc += 5;
        }
        
    }
    if(soc > 100)
    {
        soc = 100;
    }else if(soc < 0)
    {
        soc = 0;
    }

    return soc;
}

static void vol2soc_batch(uint16_t *vol, int16_t *tempra, float *soc)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        soc[i] = vol2soc(vol[i], tempra[i]);
    }
}


/**
 * @brief  init soc module, read saved soc and group soc( last power offf saved)
 */
void soc_init(void)
{
    float soc_saved[CELL_NUMS];
    float soc_saved_group;
    float soc_lookuptable[CELL_NUMS];
    float soc_firstPowerUp[CELL_NUMS];
    memset(soc_firstPowerUp, 0xff, sizeof(soc_firstPowerUp));
    bool soc_abnormal_flag[CELL_NUMS];
	memset(soc_abnormal_flag, false, sizeof(soc_abnormal_flag));
    // read saved soc(last soc before shutdown)
	bool force_save = false;
    int8_t ret = read_saved_soc(soc_saved);

    // todo ocv calibration,set init soc and soc_er2
    vol2soc_batch(g_celVol, g_celTmp, soc_lookuptable);
    if(ret == 0){
        if(memcmp(soc_saved, soc_firstPowerUp, sizeof(soc_saved)) == 0)
        {
            for(int i = 0; i < CELL_NUMS; i++)
            {
                soc_abnormal_flag[i] = true;
            }
        }else{
            for(int i = 0; i < CELL_NUMS; i++)
            {
                if(soc_saved[i] < 0 || soc_saved[i] > 100){
                    soc_abnormal_flag[i] = true;
                }
#if SOC_INIT_OCV_CAL_ENABLE
                else if(fabs(soc_saved[i]-soc_lookuptable[i]) > SOC_INIT_OCV_CAL_DIFF_THRESHOLD)
                {
                    soc_abnormal_flag[i] = true;
                }
#endif
                else{
                    soc_abnormal_flag[i] = false;
                }
            }
        }
    }else{
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            soc_abnormal_flag[i] = true;
        }
    }

	
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        if(soc_abnormal_flag[i]){
			force_save = true;
            g_socInfo[i].soc = soc_lookuptable[i];
            g_socInfo[i].socEr2 = SOC0_ER2_LOOKUP_TABLE;
        }else{
            g_socInfo[i].soc = soc_saved[i];
            g_socInfo[i].socEr2 = SOC0_ER2_SAVED;
        }
        g_celSOC[i] = g_socInfo[i].soc;
    }

    // read saved group soc(last soc before shutdown)
    ret = read_saved_soc_group(&soc_saved_group);
    g_grpSOC = soc_saved_group;
    float sum_soc = 0;
    float avg_soc = 0;
    float unsorted_soc[CELL_NUMS];
    float sorted_soc[CELL_NUMS];
    for(int i = 0; i < CELL_NUMS; i++)
    {
        unsorted_soc[i] = g_socInfo[i].soc;
        sum_soc += g_socInfo[i].soc;
    }    
    avg_soc = sum_soc/CELL_NUMS;
    bubbleSort_ascend_float(unsorted_soc, sorted_soc, CELL_NUMS);
    if(ret == 0){
        if(soc_saved_group < sorted_soc[0] || soc_saved_group > sorted_soc[CELL_NUMS-1])
        {
            g_grpSOC = avg_soc;
        }
    }else{
        g_grpSOC = avg_soc;
    }



	soc_save(force_save);

    for(int i = 0; i < CELL_NUMS; i++)
    {
        g_socInfo[i].soc_smooth = 0;
    }
    last_group_state = GROUP_STATE_standby;
    standby_hold_time = 0;
    trans_dsg_acc_time = 0;
    trans_chg_acc_time = 0;
    
}

/**
 * @param force give true to force save
 */
void soc_save(bool force)
{
    if(!force){
        static uint32_t last_save = 0;
        static bool save = false;
        static bool grp_save = false;
        static float lastsoc[CELL_NUMS];
        static float last_grpsoc;
        // cell soc save check
        for(int i = 0; i < CELL_NUMS; i++)
        {
            if(fabsf(g_celSOC[i]-lastsoc[i]) > SOC_SAVE_DIFF_PERCENT)
            {
                save = true;
            }
        }
        // group soc save check
        if(fabsf(g_grpSOC-last_grpsoc) > SOC_GRP_SAVE_DIFF_PERCENT)
        {
            grp_save = true;
        }

        uint32_t now = timebase_get_time_s();

        if(now- last_save > SOC_SAVE_INTERVAFL)
        {
            last_save = now;
            if(save){
                for(int i = 0; i < CELL_NUMS; i++)
                {
                    lastsoc[i] = g_celSOC[i];
                }
                DEBUG_LOG("soc save\n");
                write_saved_soc(lastsoc);

                save = false;
            }
            if(grp_save){
                last_grpsoc = g_grpSOC;
                write_saved_soc_group(g_grpSOC);
                grp_save = false;
            }
        }   
    }else{
        float soc_write[CELL_NUMS];
        for(int i = 0; i < CELL_NUMS; i++)
        {
            soc_write[i] = g_celSOC[i];
        }

        write_saved_soc(soc_write);

        write_saved_soc_group(g_grpSOC);
    }


    
}






/**
 * @brief  soc task , need call priodically
 * @param full give true when full charge, false when not
 * @param empty give true when empty charge, false when not
 */
void soc_task(bool full, bool empty)
{
    enum GroupState state = check_current_group_state(g_cur);
#if SOX_DEBUG
    static uint32_t callCount = 0;
    callCount++;
    UNUSED(callCount);      // avoid warning in debug mode
#endif
    for (size_t i = 0; i < CELL_NUMS; i++)
    {

        mysoc(&g_socInfo[i], g_cur, g_celVol[i], g_celTmp[i], g_celSOH[i]);

        // output soc
        if(g_socInfo[i].soc_smooth>0)         // smooth enabled
        {
            if(g_cur>=0 &&  g_socInfo[i].soc_smooth>g_socInfo[i].soc){
                g_celSOC[i] = g_socInfo[i].soc_smooth;
              DEBUG_LOG("use smooth soc\n");
            }else if(g_cur<=0 &&  g_socInfo[i].soc_smooth<g_socInfo[i].soc){
                g_celSOC[i] = g_socInfo[i].soc_smooth;
              DEBUG_LOG("use smooth soc\n");
            }else{
                g_celSOC[i] = g_socInfo[i].soc;
            }
        }
        else{
            g_celSOC[i] = g_socInfo[i].soc;
        }
    
    }


    
    if(state == GROUP_STATE_standby)
    {
        static uint32_t s_standby_timeCount = 0;
        s_standby_timeCount ++;
        if(s_standby_timeCount > 10*60)
        {
            for (size_t i = 0; i < CELL_NUMS; i++)
            {
                g_socInfo[i].pureAH_lock = false;
                if(g_socInfo[i].soc_smooth>0 && g_socInfo[i].soc_smooth < g_socInfo[i].soc){
                    g_socInfo[i].soc = g_socInfo[i].soc_smooth;
                    g_socInfo[i].soc_smooth = 0;
                }
                memset(g_socInfo[i].cell_vol_buffer, 0, sizeof(g_socInfo[i].cell_vol_buffer));
                g_socInfo[i].cell_vol_buff_idx = 0;
                g_socInfo[i].swith_curve_time = 0;
                g_socInfo[i].last_curve = NULL;
            }
            s_standby_timeCount = 0;
        }
    }
	
	// XXX grp soc maybe need a mux lock when in rtos
    gropuSOC();
	
	if(full)
    {
#if SOX_GROUP_FULL_CAL_CELL
       for (size_t i = 0; i < CELL_NUMS; i++)
       {
           g_socInfo[i].soc = 100;
           g_celSOC[i] = 100;
       }
#endif 
       if(g_grpSOC < 98)
       {
            g_soh_calibrate_tigger = SOH_CALIBRATION_TIGGERED_BY_CHARGING;
            g_group_soc_before_jump = g_grpSOC;
       }
        g_grpSOC = 100;
        // reset some state
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            g_socInfo[i].pureAH_lock = false;
            g_socInfo[i].soc_smooth = 0;
            memset(g_socInfo[i].cell_vol_buffer, 0, sizeof(g_socInfo[i].cell_vol_buffer));
            g_socInfo[i].cell_vol_buff_idx = 0;
            g_socInfo[i].swith_curve_time = 0;
            g_socInfo[i].last_curve = NULL;
        }
    }
    if(empty)
    {
#if SOX_GROUP_EMPTY_CAL_CELL
       for (size_t i = 0; i < CELL_NUMS; i++)
       {
           g_socInfo[i].soc = 0;
           g_celSOC[i] = 0;
       }
#endif
       if(g_grpSOC > 2)
       {
            g_soh_calibrate_tigger = SOH_CALIBRATION_TIGGERED_BY_DISCHARGING;
            g_group_soc_before_jump = g_grpSOC;
       }
        g_grpSOC = 0;
        // reset some state
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            g_socInfo[i].pureAH_lock = false;
            g_socInfo[i].soc_smooth = 0;
            memset(g_socInfo[i].cell_vol_buffer, 0, sizeof(g_socInfo[i].cell_vol_buffer));
            g_socInfo[i].cell_vol_buff_idx = 0;
            g_socInfo[i].swith_curve_time = 0;
            g_socInfo[i].last_curve = NULL;
        }
    }
	// XXX grp soc maybe need a mux unlock when in rtos

}




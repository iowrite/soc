#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "soc.h"
#include "curve.h"
#include "sox_private.h"
#include "sox.h"
#include "port.h"
#include "sox_config.h"
#include "common.h"




#define EKF_W(diffAH, cap, cur)                 ((50.0/(DIFF_T_SEC*1000) + CUR_SAMPLE_ERR_A/cur + CAP_ERR_AH/cap+SOH_ERR_PERCENT/100.0) *  diffAH)     
#define EKF_Q(diffAH, cap, cur)                 (EKF_W(diffAH, cap, cur)*EKF_W(diffAH, cap, cur))                 
#define EKF_R_1                                 (VOL_SAMPLE_ERR_MV_1*VOL_SAMPLE_ERR_MV_1)
#define EKF_R_2                                 (VOL_SAMPLE_ERR_MV_2*VOL_SAMPLE_ERR_MV_2)
#define EKF_R_3                                 (VOL_SAMPLE_ERR_MV_3*VOL_SAMPLE_ERR_MV_3)


#define SOC0_ER2_SAVED                      100             // 10% error
#define SOC0_ER2_LOOKUP_TABLE               900             // 30% error



struct SOC_Info g_socInfo[CELL_NUMS];






static const uint16_t get_cap(float cur, uint16_t tempra)
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
        int cidx = round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        if(s_cap_list_chg[tidx][cidx] == 0){
            assert(0);
        }
        return s_cap_list_chg[tidx][cidx];

    }else if(cur < 0)
    {
        float c = cur/100*10;
        int cidx = -round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        if(s_cap_list_dsg[tidx][cidx] == 0){
            assert(0);
        }
        return s_cap_list_dsg[tidx][cidx];
    }



    return 0;
}




static const uint16_t * get_v(const uint16_t *chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], const uint16_t *dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], float cur, uint16_t tempra)
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
        int cidx = round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        assert(chg_curve[tidx][cidx] != NULL);
        return chg_curve[tidx][cidx];

    }else if(cur < 0)
    {
        float c = cur/100*10;
        int cidx = -round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        assert(dsg_curve[tidx][cidx] != NULL);
        return dsg_curve[tidx][cidx];
    }
    return NULL;
}


static const int16_t * get_k(const int16_t *chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], const int16_t *dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], float cur, uint16_t tempra)
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
        int cidx = round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        assert(chg_curve[tidx][cidx] != NULL);
        return chg_curve[tidx][cidx];

    }else if(cur < 0)
    {
        float c = cur/100*10;
        int cidx = -round(c)-1;
        if(cidx < 0)
        {
            cidx = 0;
        }
        if(cidx > 4)
        {
            cidx = 4;
        }
        assert(dsg_curve[tidx][cidx] != NULL);
        return dsg_curve[tidx][cidx];
    }
    return NULL;
}



static const uint16_t * get_curve_v(float cur, uint16_t tempra)
{
    return get_v(s_chg_curve, s_dsg_curve, cur, tempra);
}

static const int16_t * get_curve_k(float cur, uint16_t tempra)
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

enum GroupState check_current_group_state(float cur)
{
    static enum GroupState last_group_state = GROUP_STATE_standby;
    static uint32_t standby_hold_time = 0;
    static uint32_t trans_dsg_acc_time = 0;
    static uint32_t trans_chg_acc_time = 0;
    // if(trans_chg_acc_time == 132){
    //     printf("herer\n");
    // }
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
        if(fabs(cur) < CUR_WINDOW_A){                               // reset state logic
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
        if(fabs(cur)<CUR_WINDOW_A){
            g_group_state = GROUP_STATE_standby;
            standby_hold_time = timebase_get_time_s();
            last_group_state = GROUP_STATE_charging;
        }else if(cur<-CUR_WINDOW_A){
            g_group_state = GROUP_STATE_transfer;
            last_group_state = GROUP_STATE_charging;
        }
        break;
    case GROUP_STATE_discharging:
        if(fabs(cur)<CUR_WINDOW_A){
            g_group_state = GROUP_STATE_standby;
            standby_hold_time = timebase_get_time_s();
            last_group_state = GROUP_STATE_discharging;
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


void mysoc_pureAH(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra, float soh)
{
    const uint16_t cap = get_cap(cur, tempra);

    const float capf = cap/10.0*(soh/100);

    float diffAH = DIFF_T_SEC/3600.0*cur/capf*100;
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





void mysoc_smooth(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra, float soh)
{
    static int callcount = 0;
    callcount++;
    // if(callcount == 6018){
    //     printf("fds\n");
    // }
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
    


    if(cur > 0 && vol > SOC_SMOOTH_START_VOL_CHG || cur < 0 && vol < SOC_SMOOTH_START_VOL_DSG)
    {
        if(SOCinfo->soc_smooth == 0){
            SOCinfo->soc_smooth = SOCinfo->soc;
        }

        if(cur>0  && ((int)newest_vol-(int)oldest_vol)){
            int smooth_full_estimate_s = (*g_chg_stop_vol-vol)/(newest_vol-oldest_vol)*(CELL_VOL_BUFFER_LEN-1)*CELL_VOL_BUFFER_SAMPLE_TIME_S;
            
            
        }else if(cur<0 && ((int)oldest_vol-(int)newest_vol)){
            int smooth_empty_estimate_s = (vol-*g_dsg_stop_vol)/(oldest_vol-newest_vol)*(CELL_VOL_BUFFER_LEN-1)*CELL_VOL_BUFFER_SAMPLE_TIME_S;
            float smoothDiff = SOCinfo->soc_smooth/smooth_empty_estimate_s*(3+15*(vol-*g_dsg_stop_vol)/(SOC_SMOOTH_START_VOL_DSG-*g_dsg_stop_vol));
            SOCinfo->soc_smooth -= smoothDiff;
//            if(callcount%16 == 8 && SOCinfo->soc_smooth<1){
//                printf("dsg soc speedup %f,smooth_full_estimate_s: %d, soc_smooth %f\r\n", smoothDiff, smooth_empty_estimate_s, SOCinfo->soc_smooth);
//            }
        }
    }

    if(SOCinfo->soc_smooth < 0)
    {
        SOCinfo->soc_smooth = 0.1;             // not equal to 0(zero mean smooth not enable)
    }else if(SOCinfo->soc_smooth > 100)
    {
        SOCinfo->soc_smooth = 100;
    }

}


float getEKF_Q(float soc, float cur)
{
    if(cur > 0)
    {
        return (soc/100 + (soc/100*MAX_EKF_Q_PERCENT)*(soc/100*MAX_EKF_Q_PERCENT));
    }else{
        return (soc/100 + ((100-soc)/100*MAX_EKF_Q_PERCENT)*((100-soc)/100*MAX_EKF_Q_PERCENT));
    }

    return nan("");
}





static uint32_t getEKF_R(float H_soc, uint16_t vol, const uint16_t *curve, const int16_t *curveK, uint16_t switch_curve_time)
{

    float vol_R;
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
    float H;
    if(index == 0)
    {
        H = curveK[0];
    }else if(index == SOC_POINT_NUM)
    {
        H = curveK[SOC_POINT_NUM-1];
    }else{
        H = (curveK[index-1] + (curveK[index]-curveK[index-1])*(vol-curve[index-1])/(curve[index]-curve[index-1]))/10.0;
    }

    if(H > H_soc)
    {
        H = H_soc;
    }

    float H_R;
    if(H<2)
    {
        int t = abs(VOL_SAMPLE_ERR_MV_1-VOL_SAMPLE_ERR_MV_2);
        H_R = (VOL_SAMPLE_ERR_MV_1+(2-H)*t)*(VOL_SAMPLE_ERR_MV_1+(2-H)*t);
    }else if(H>4)
    {
        H_R = VOL_SAMPLE_ERR_MV_3*VOL_SAMPLE_ERR_MV_3;
    }else{
        int t = abs(VOL_SAMPLE_ERR_MV_3-VOL_SAMPLE_ERR_MV_1);
        H_R = (VOL_SAMPLE_ERR_MV_3+(H-2)*t)*(VOL_SAMPLE_ERR_MV_3+(H-2)*t);
    }
    if(switch_curve_time){
        return H_R+40000*switch_curve_time/600+10000; 
    }else{
        return H_R;
    }
    
  
}



void mysocEKF(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra, float soh)
{
    
    static int callCount = 0;
    callCount++;
    static float pureAHSUM = 0;
    // if(callCount == 165680)
    // {
    //     printf("fds\n");

    // }

    // if(callCount%16 == 2)
    // {
    //     printf("fds\n");
    // }
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


    // if(callCount %16 == 12 && SOCinfo->swith_curve_time)
    // {
    //     printf("callCount: %d, switch count : %d \n", callCount/16, SOCinfo->swith_curve_time);
    // }
    // if(callCount %16 == 1 && SOCinfo->swith_curve_time)
    // {
    //     printf("callCount: %d\n", callCount/16);
    // }

    const float capf = cap/10.0*(soh/100);

    float diffAH = DIFF_T_SEC/3600.0*cur/capf*100;
    float SOCcal = SOCinfo->soc + diffAH;
    float Q = getEKF_Q(SOCcal, cur);
    float SOCer2Cal = SOCinfo->socEr2 + Q;
    pureAHSUM += diffAH;
    // printf("diffAH : %f, EKF_W : %f pureAH: %f\n", diffAH, EKF_W(diffAH,capf, cur), pureAHSUM);
    float H = 0, Hprev = 0, Hnext = 0;
    float estVol = 0, estVolPrev = 0, estVolNext = 0;
    float SOCerCal = sqrt(SOCer2Cal);
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
        if(SOCcal < 5)
        {
            float sock = SOCcal-(int)SOCcal;
            estVolPrev = curve[(int)SOCcal];
            estVolNext = curve[(int)SOCcal+1];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }else if(SOCcal < 95)
        {
            float sock = ((int)SOCcal%5+SOCcal-(int)SOCcal)/5;
            estVolPrev = curve[(int)SOCcal/5+4];
            estVolNext = curve[(int)SOCcal/5+1+4];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }    
        else{
            float sock = SOCcal-(int)SOCcal;
            estVolPrev = curve[23+(int)SOCcal-95];
            estVolNext = curve[23+(int)SOCcal-95+1];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }
        //printf("callcount %d soccal : %f  estVolPrev :%f  estVol : %f  estVolNext :%f \n", callCount, SOCcal, estVolPrev, estVol, estVolNext);

        if(SOCcal < 5)
        {
            float Hk = SOCcal-(int)SOCcal;
            Hprev = (float)curveK[((int)SOCcal)]/10;
            Hnext = (float)curveK[(int)SOCcal+1]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }else if(SOCcal < 95)
        {
            float Hk = ((int)SOCcal%5+SOCcal-(int)SOCcal)/5;
            Hprev = (float)curveK[((int)SOCcal/5+4)]/10;
            Hnext = (float)curveK[(int)SOCcal/5+1+4]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }    
        else{
            float Hk = SOCcal-(int)SOCcal;
            Hprev = (float)curveK[23+(int)SOCcal-95]/10;
            Hnext = (float)curveK[23+(int)SOCcal-95+1]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }
        // printf("callcount %d hprev :%f  H : %f  hnext :%f \n", callCount, Hprev, H, Hnext);
    }
    float K = 0;
    // if(callCount%16 == 13 && SOCcal >= 92)
    // {
    //     printf("fdfsa\n");
    // }
    uint32_t ekfR = getEKF_R(H, vol, curve, curveK, SOCinfo->swith_curve_time);


    if(SOCer2Cal>Q)
    {
        SOCer2Cal = Q;
    }
    K = SOCer2Cal*H/(H*SOCer2Cal*H+ekfR);
    // if(callCount%16 == 13){
    //     printf("soc: %f, R: %d, k: %f\n", SOCinfo->soc, ekfR, K);
    // }
    
    float res = SOCcal+K*((float)vol-estVol);
    // printf("callcount %d  H: %f K :%f  kcal : %f , vol: %d, estvol: %lf\n", callCount, H, K, K*((float)vol-estVol), vol, estVol);
    float resEr2 = (1-K*H)*SOCer2Cal;
    if(resEr2 > Q)
    {
        resEr2 = Q;
    }
    float SOCerRes = sqrt(resEr2);


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
            //res = SOCinfo->soc;
            // resEr2 = SOCinfo->socEr2;
            // printf("soc underflow\n");
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
            //res = SOCinfo->soc;
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
    // printf("soc : %f \n", 100-res);
    //if(callCount%16 == 0){
    //    printf("%d soc error2: %f Q:%f R:%d, H:%f, K:%lf kcal : %f , vol: %d, estvol: %lf\n",callCount/16, SOCinfo->socEr2, Q, ekfR, H, K, K*((float)vol-estVol), vol, estVol);
    //}
    
}






void mysoc(struct SOC_Info *SOCinfo, float cur, uint16_t vol, int16_t tempra, float soh)
{
    if(fabs(cur) > CUR_WINDOW_A)
    {
        if((tempra<0 && tempra > -200) || (tempra < PURE_AH_LOCK_TEMP_THRESHOLD && fabs(cur) > PURE_AH_LOCK_CUR_THRESHOLD)){
            SOCinfo->pureAH_lock = true;
        }
        if(SOCinfo->pureAH_lock)
        {
            mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);
            mysoc_smooth(SOCinfo, cur, vol, tempra, soh);
//            printf("pureAH lock\n");
        }else{
            if(g_group_state == GROUP_STATE_transfer){
                mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);               // Ampere-hour Integration only
            }else{
                mysocEKF(SOCinfo, cur, vol, tempra, soh);                   // Ampere-hour Integration + EKF
            }
        }
    }




}


/* bubble sort : ascending */
static void bubbleSort_ascend(uint16_t *inputArr, uint16_t *outputArr, uint16_t size)
{
    memcpy(outputArr, inputArr, size*sizeof(uint16_t));
    for (size_t i = 0; i < size-1; i++)
    {
        for (size_t j = 0; j < size-i-1; j++)
        {
            if(outputArr[j] > outputArr[j+1])
            {
                uint16_t tmp = outputArr[j];
                outputArr[j] = outputArr[j+1];
                outputArr[j+1] = tmp;
            }
        }
    }
}

static void bubbleSort_ascend_float(float *inputArr, float *outputArr, uint16_t size)
{
    memcpy(outputArr, inputArr, size*sizeof(float));
    for (size_t i = 0; i < size-1; i++)
    {
        for (size_t j = 0; j < size-i-1; j++)
        {
            if(outputArr[j] > outputArr[j+1])
            {
                float tmp = outputArr[j];
                outputArr[j] = outputArr[j+1];
                outputArr[j+1] = tmp;
            }
        }
    }
}

static void bubbleSort_ascend_duble(float *inputArr, float *outputArr, uint16_t size)
{
    memcpy(outputArr, inputArr, size*sizeof(float));
    for (size_t i = 0; i < size-1; i++)
    {
        for (size_t j = 0; j < size-i-1; j++)
        {
            if(outputArr[j] > outputArr[j+1])
            {
                float tmp = outputArr[j];
                outputArr[j] = outputArr[j+1];
                outputArr[j+1] = tmp;
            }
        }
    }
}



static void gropuSOC()
{
    static int callCount = 0;
    callCount++;
    float unsortedSOC[CELL_NUMS];
    for(int i = 0; i < CELL_NUMS; i++)
    {
        unsortedSOC[i] = g_socInfo[i].soc;
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

    // if(callCount == 3560){
    //     printf("change\n");
    // }


    // uint16_t hSOC = maxSOC*(maxSOC/100.0)+minSOC*(1-maxSOC/100.0);
    // uint16_t lSOC= minSOC*(1-minSOC/100.0)+maxSOC*(minSOC/100.0);

    
    // uint16_t h = hSOC>lSOC?hSOC:lSOC;
    // uint16_t l = lSOC<hSOC?lSOC:hSOC;
    uint16_t grpsoc;
    // uint16_t grpsoc = round((*g_grpSOC)/100.0*hSOC+(100-*g_grpSOC)/100.0*lSOC);

#define GRP_Q_MAX               25
#define GRP_Q_Min               1
#define GRP_Q                   0.01f

#define  E_HIGH_MIN       1
#define  E_HIGH_MAX       5
#define  E_LOW_MIN        1
#define  E_LOW_MAX        5


#define  R_HIGH_MIN       (E_HIGH_MIN*E_HIGH_MIN)
#define  R_HIGH_MAX       (E_HIGH_MAX*E_HIGH_MAX)
#define  R_LOW_MIN        (E_LOW_MIN*E_LOW_MIN)
#define  R_LOW_MAX        (E_LOW_MAX*E_LOW_MAX)
#define  R_AVG_MIN        (E_AEG_MIN*E_AEG_MIN)
#define  R_AVG_MAX        (E_AEG_MIN*E_AEG_MIN)


    static float avg_soc_buff[10];
    static uint8_t buff_idx = 0;


    static uint32_t last_record_time = 0;
    if(timebase_get_time_s() - last_record_time > (uint32_t)60)
    {
        last_record_time = timebase_get_time_s();
        avg_soc_buff[buff_idx] = avgSOC;
        buff_idx++;
        if(buff_idx >= 10)
        {
            buff_idx = 0;
        }
    }

    float last_avg_soc;
    if(buff_idx == 9)
    {
        last_avg_soc = avg_soc_buff[0];
    }else
    {
        last_avg_soc = avg_soc_buff[buff_idx+1];
    }

    if(fabs(*g_cur)>CUR_WINDOW_A){

        float max_soc_change_R_offset = fabs(maxSOC - last_avg_soc);
        float min_soc_change_R_offset = fabs(minSOC - last_avg_soc);
        float max_soc_change_R_offset2 = max_soc_change_R_offset*max_soc_change_R_offset;
        float min_soc_change_R_offset2 = min_soc_change_R_offset*min_soc_change_R_offset;
        if(max_soc_change_R_offset2 > 25)
        {
            max_soc_change_R_offset2 = 25; 
        }
        if(min_soc_change_R_offset2 > 25)
        {
            min_soc_change_R_offset = 25; 
        }

        float maxSOC_R = R_HIGH_MIN + (1-*g_grpSOC/100.0f) * (R_HIGH_MAX- R_HIGH_MIN) + max_soc_change_R_offset2;
        float minSOC_R = R_LOW_MIN + *g_grpSOC/100.0f * (R_LOW_MAX- R_LOW_MIN) + min_soc_change_R_offset2;


        if(*g_grpSOC > 50)
        {
            if(minSOC_R > 25)
            {
                minSOC_R = 25;
            }
            if(maxSOC_R > minSOC_R){
                maxSOC_R = minSOC_R -1;
            }
        }else if(*g_grpSOC < 50)
        {
            if(maxSOC_R > 25)
            {
                maxSOC_R = 25;
            }
            if(minSOC_R > maxSOC_R)
            {
                minSOC_R = maxSOC_R -1;
            }
        }


        //printf("last_avg_soc %f, maxSOC_R: %f, minSOC_R: %f, maxsoc %f, minsoc %f", last_avg_soc, maxSOC_R, minSOC_R, maxSOC, minSOC);
        static float cal_grp_soc;
        static bool grp_soc_init = false;
        if(!grp_soc_init){
            grp_soc_init = true;
            cal_grp_soc = *g_grpSOC;
        }
        

        static float grp_soc_p = 1;

        float cal_grp_soc_p=grp_soc_p + GRP_Q;

        //printf("kf soc: %f, p: %f \n", cal_grp_soc, grp_soc_p);

        float H[2][1] = {1, 1};
        float H_t[1][2] = {1, 1};
        float Z[2][1] = {minSOC, maxSOC};
        float R[2][2] = {    minSOC_R,       1       ,
                                1       ,       maxSOC_R,
                            };
        float S[2][2] = {0};
        float H_P[2][1] = {cal_grp_soc_p, cal_grp_soc_p};
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

        float P_H_t[1][2] = {cal_grp_soc_p, cal_grp_soc_p};

        float K[1][2] = {0};
        matrix_multiply((float *)P_H_t, (float *)S_i, (float *)K, 1, 2, 2);


        float Z_H[2][1] = {0};
        float H_x[2][1] = {cal_grp_soc, cal_grp_soc};
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


        grpsoc = round(cal_grp_soc);








        if(grpsoc > 99)
        {
            grpsoc = 99;
        }else if(grpsoc < 1){
            grpsoc = 1;
        }

        static uint32_t smooth_count = 0;
        uint16_t smooth_soc = *g_grpSOC;
        if(abs(grpsoc - *g_grpSOC)>=2)
        {        
            if(timebase_get_time_s()-smooth_count > (uint32_t)2)
            {
                smooth_count = timebase_get_time_s();
                if(grpsoc > *g_grpSOC)
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
                *g_grpSOC = smooth_soc;

            }
        }else{
            *g_grpSOC = grpsoc;
        }
        // printf("call: %d, hSOC:%d, lSOC:%d, grpSOC:%d\n",callCount, hSOC, lSOC, *g_grpSOC);
    }
}

/**
 * @brief  convert voltage to soc by lookup table
 * @return soc
 */
static float vol2soc(uint16_t vol, int16_t tempra)
{
    float soc = 0;
    
    for(int i = 0; i < OCV_POINT_NUM; i++)
    {
        if(vol > temp_ocv[4][i])
        {
            soc += 5;
        }
        
    }

    return soc-5;
}

static void vol2soc_batch(uint16_t *vol, int16_t *tempra, float *soc)
{
    for(int i = 0; i < CELL_NUMS; i++)
    {
        soc[i] = vol2soc(vol[i], tempra[i]);
    }
}

void soc_init()
{
    float soc_saved[CELL_NUMS];
    float soc_saved_group;
    float soc_lookuptable[CELL_NUMS];
    float soc_firstPowerUp[CELL_NUMS];
    memset(soc_firstPowerUp, 0xff, sizeof(soc_firstPowerUp));
    bool soc_abnormal_flag[CELL_NUMS];
	memset(soc_abnormal_flag, false, sizeof(soc_abnormal_flag));
    // read saved soc(last soc before shutdown)
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
                else if(fabs(soc_saved[i]-soc_lookuptable[i]) > 30)
                {
                    soc_abnormal_flag[i] = true;
                }else{
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
            g_socInfo[i].soc = soc_lookuptable[i];
            g_socInfo[i].socEr2 = SOC0_ER2_LOOKUP_TABLE;
        }else{
            g_socInfo[i].soc = soc_saved[i];
            g_socInfo[i].socEr2 = SOC0_ER2_SAVED;
        }
    }

    // read saved group soc(last soc before shutdown)
    ret = read_saved_soc_group(&soc_saved_group);
    *g_grpSOC = soc_saved_group;
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
            *g_grpSOC = round(avg_soc);
        }
    }else{
        *g_grpSOC = round(avg_soc);
    }


    port_soc_init();
	
	port_soc_output();
	soc_save(true);

    
}

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
            if(fabs(g_socInfo[i].soc-lastsoc[i]) > SOC_SAVE_DIFF_PERCENT)
            {
                save = true;
            }
        }
        // group soc save check
        if(fabs(*g_grpSOC-last_grpsoc) > SOC_GRP_SAVE_DIFF_PERCENT)
        {
            grp_save = true;
        }

        uint32_t now = timebase_get_time_s();
        // printf("now:%d, last:%d\n", now, last_save);
        if(now- last_save > SOC_SAVE_INTERVAFL)
        {
            last_save = now;
            if(save){
                for(int i = 0; i < CELL_NUMS; i++)
                {
                    lastsoc[i] = g_socInfo[i].soc;
                }
                // printf("soc save\n");
                write_saved_soc(lastsoc);
                save = false;
            }
            if(grp_save){
                last_grpsoc = *g_grpSOC;
                write_saved_soc_group(*g_grpSOC);
                grp_save = false;
            }
        }   
    }else{
        float soc_write[CELL_NUMS];
        for(int i = 0; i < CELL_NUMS; i++)
        {
            soc_write[i] = g_socInfo[i].soc;
        }

        write_saved_soc(soc_write);
        write_saved_soc_group(*g_grpSOC);
    }


    
}







void soc_task(bool full, bool empty)
{
    port_soc_input();


    enum GroupState state = check_current_group_state(*g_cur);
    static uint32_t callCount = 0;
    callCount++;
    // if(callCount == 132)
    // {
    //     printf("fdsa\n");
    // }
    
    for (size_t i = 0; i < CELL_NUMS; i++)
    {

        mysoc(&g_socInfo[i], *g_cur, g_celVol[i], g_celTmp[i], g_celSOH[i]);

        // output soc
        if(g_socInfo[i].soc_smooth)
        {
            if(*g_cur>0 &&  g_socInfo[i].soc_smooth>g_socInfo[i].soc){
                g_celSOC[i] = g_socInfo[i].soc_smooth;
//               printf("use smooth soc\n");
            }else if(*g_cur<0 &&  g_socInfo[i].soc_smooth<g_socInfo[i].soc){
                g_celSOC[i] = g_socInfo[i].soc_smooth;
//               printf("use smooth soc\n");
            }else{
                g_celSOC[i] = round(g_socInfo[i].soc);
            }
        }
        else{
            g_celSOC[i] = round(g_socInfo[i].soc);
        }
        

        // if(i == 0)
        // {
            // printf("soc error2: %f \n", g_socInfo[0].socEr2);
        // }
    }
    //printf("call: %d, cur: %f, state: %d\n",callCount, *g_cur, g_group_state);

    
    if(state == GROUP_STATE_standby)
    {
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
	
	// XXX grp soc maybe need a mux lock when in rtos
    gropuSOC();
	
	if(full)
    {
//        for (size_t i = 0; i < CELL_NUMS; i++)
//        {
//            g_socInfo[i].soc = 100;
//            g_celSOC[i] = 100;
//        }
        *g_grpSOC = 100;
    }
    if(empty)
    {
//        for (size_t i = 0; i < CELL_NUMS; i++)
//        {
//            g_socInfo[i].soc = 0;
//            g_celSOC[i] = 0;
//        }
        *g_grpSOC = 0;
    }
	// XXX grp soc maybe need a mux unlock when in rtos
	
    //printf("call: %d, grpsoc: %d\n", callCount, *g_grpSOC);
    port_soc_output();

}




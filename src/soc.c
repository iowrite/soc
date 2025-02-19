#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "soc.h"
#include "curve.h"
#include "sox_private.h"
#include "sox.h"
#include "port.h"
#include "sox_config.h"




 #define EKF_W(diffAH, cap, cur)                 ((50.0/(DIFF_T_SEC*1000) + CUR_SAMPLE_ERR_A/cur + CAP_ERR_AH/cap+SOH_ERR_PERCENT/100.0) *  diffAH)     
#define EKF_Q(diffAH, cap, cur)                 (EKF_W(diffAH, cap, cur)*EKF_W(diffAH, cap, cur))                 
#define EKF_R_1                                 (VOL_SAMPLE_ERR_MV_1*VOL_SAMPLE_ERR_MV_1)
#define EKF_R_2                                 (VOL_SAMPLE_ERR_MV_2*VOL_SAMPLE_ERR_MV_2)
#define EKF_R_3                                 (VOL_SAMPLE_ERR_MV_3*VOL_SAMPLE_ERR_MV_3)

#define SOC0                                100
#define SOC0_ER2                            25
#define SOC0_ER2_SAVED                      100             // 10% error
#define SOC0_ER2_LOOKUP_TABLE               900             // 30% error


struct SOC_Info
{
    bool pureAH_lock;
    uint8_t cell_vol_buff_idx;
    uint16_t cell_vol_buffer[CELL_VOL_BUFFER_LEN];
    uint32_t record_time;
    float oldest_vol; 
    float newest_vol;
    float soc_smooth;
    float soc;
    float socEr2;

};
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
    static uint32_t transfer_start_time = 0;
    static uint32_t standby_hold_time = 0;
    switch (g_group_state)
    {
    case GROUP_STATE_standby:
        if(cur > CUR_WINDOW_A && last_group_state == GROUP_STATE_standby)               // initial state is standby
        {
            g_group_state = GROUP_STATE_charging;
        }else if(cur < -CUR_WINDOW_A && last_group_state == GROUP_STATE_standby)
        {
            g_group_state = GROUP_STATE_discharging;                                    // initial state is standby
        }
        if(timebase_get_time_s()-standby_hold_time < STANDBY_HOLD_TIME){
            if(cur>CUR_WINDOW_A && last_group_state == GROUP_STATE_discharging){
                g_group_state = GROUP_STATE_transfer;
                transfer_start_time = timebase_get_time_s();
            }else if(cur<-CUR_WINDOW_A && last_group_state == GROUP_STATE_charging){
                g_group_state = GROUP_STATE_transfer;
                transfer_start_time = timebase_get_time_s();
            }
        }else{
            standby_hold_time = timebase_get_time_s();
        }
        break;
    case GROUP_STATE_transfer:
        if(fabs(cur) < CUR_WINDOW_A){                               // reset state logic
            g_group_state = GROUP_STATE_standby;
            last_group_state = GROUP_STATE_standby;
        } 
        else if(last_group_state == GROUP_STATE_charging)
        {
            if(cur > CUR_WINDOW_A && timebase_get_time_s()-transfer_start_time > GROUP_STATE_CHANGE_TIME){
                g_group_state = GROUP_STATE_charging;
                last_group_state = GROUP_STATE_transfer;
            }
        }else if(last_group_state == GROUP_STATE_discharging)
        {
            if(cur < -CUR_WINDOW_A && timebase_get_time_s()-transfer_start_time > GROUP_STATE_CHANGE_TIME){
                g_group_state = GROUP_STATE_charging;
                last_group_state = GROUP_STATE_transfer;
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
            transfer_start_time = timebase_get_time_s();
        }
        break;
    case GROUP_STATE_discharging:
        if(fabs(cur)<CUR_WINDOW_A){
            g_group_state = GROUP_STATE_standby;
            standby_hold_time = timebase_get_time_s();
            last_group_state = GROUP_STATE_discharging;
        }else if(cur>CUR_WINDOW_A){
            g_group_state = GROUP_STATE_transfer;
            last_group_state = GROUP_STATE_charging;
            transfer_start_time = timebase_get_time_s();
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

    const double capf = cap/10.0*(soh/100);

    double diffAH = DIFF_T_SEC/3600.0*cur/capf*100;
    double SOCcal = SOCinfo->soc + diffAH;
    double SOCer2Cal = SOCinfo->socEr2 + EKF_Q(diffAH,capf, cur);



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
    if(callcount == 6018){
        printf("fds\n");
    }
    if(timebase_get_time_s()-SOCinfo->record_time > CELL_VOL_BUFFER_SAMPLE_TIME_S){
            SOCinfo->record_time = timebase_get_time_s();
            SOCinfo->cell_vol_buffer[SOCinfo->cell_vol_buff_idx] = vol;
            SOCinfo->cell_vol_buff_idx++;
            if(SOCinfo->cell_vol_buff_idx >= CELL_VOL_BUFFER_LEN){
                SOCinfo->cell_vol_buff_idx = 0;
            }
    }
    if(SOCinfo->cell_vol_buff_idx == 1){
        SOCinfo->newest_vol = SOCinfo->cell_vol_buffer[0];
        SOCinfo->oldest_vol = SOCinfo->cell_vol_buffer[CELL_VOL_BUFFER_LEN-1];
    }else if(SOCinfo->cell_vol_buff_idx == 0){
        SOCinfo->newest_vol = SOCinfo->cell_vol_buffer[CELL_VOL_BUFFER_LEN-1];
        SOCinfo->oldest_vol = SOCinfo->cell_vol_buffer[CELL_VOL_BUFFER_LEN-2];
    }else{
        SOCinfo->newest_vol = SOCinfo->cell_vol_buffer[SOCinfo->cell_vol_buff_idx-1];
        SOCinfo->oldest_vol = SOCinfo->cell_vol_buffer[SOCinfo->cell_vol_buff_idx-2];
    }
    


    if(cur > 0 && SOCinfo->soc > SOC_SMOOTH_START_POINT_CHG || cur < 0 && SOCinfo->soc < SOC_SMOOTH_START_POINT_DSG)
    {
        if(SOCinfo->soc_smooth == 0){
            SOCinfo->soc_smooth = SOCinfo->soc;
        }
        const uint16_t cap = get_cap(cur, tempra);

        const double capf = cap/10.0*(soh/100);

        double diffAH = DIFF_T_SEC/3600.0*cur/capf*100;

        if(cur>0  && ((int)SOCinfo->newest_vol-(int)SOCinfo->oldest_vol)){
            int smooth_full_estimate_s = (*g_chg_stop_vol-vol)/(SOCinfo->newest_vol-SOCinfo->oldest_vol)*(CELL_VOL_BUFFER_LEN-1)*CELL_VOL_BUFFER_SAMPLE_TIME_S;
            int AH_s = (100-SOCinfo->soc_smooth)*(capf/100)/cur*3600;
            if(AH_s > smooth_full_estimate_s){
                float speedup_k = (float)AH_s/smooth_full_estimate_s-1; 
                SOCinfo->soc_smooth += speedup_k*diffAH;
                printf("chg soc speedup\r\n");
            }else{
                SOCinfo->soc_smooth = SOCinfo->soc;
                printf("chg -> AH_s: %d, smooth_full_estimate_s: %d\n", AH_s, smooth_full_estimate_s);
            }   
        }else if(cur<0 && ((int)SOCinfo->oldest_vol-(int)SOCinfo->newest_vol)){
            int smooth_empty_estimate_s = (vol-*g_dsg_stop_vol)/(SOCinfo->oldest_vol-SOCinfo->newest_vol)*(CELL_VOL_BUFFER_LEN-1)*CELL_VOL_BUFFER_SAMPLE_TIME_S;
            int AH_s = SOCinfo->soc_smooth*(capf/100)/fabs(cur)*3600;
            if(AH_s > smooth_empty_estimate_s){
                float speedup_k = (float)AH_s/smooth_empty_estimate_s-1; 
                SOCinfo->soc_smooth += speedup_k*diffAH;
                if(callcount%16 == 8){
                    printf("dsg soc speedup %f, dsg -> AH_s: %d, smooth_full_estimate_s: %d, soc_smooth %f\r\n", speedup_k, AH_s, smooth_empty_estimate_s, SOCinfo->soc_smooth);
                }
            }else{
                SOCinfo->soc_smooth = SOCinfo->soc;
                if(callcount%16 == 8){
                    printf("callcount:%d, dsg -> AH_s: %d, smooth_full_estimate_s: %d\n",callcount, AH_s, smooth_empty_estimate_s);
                }
            }  
        }
    }

    if(SOCinfo->soc_smooth < 0)
    {
        SOCinfo->soc_smooth = 0;
    }else if(SOCinfo->soc_smooth > 100)
    {
        SOCinfo->soc_smooth = 100;
    }

}


double getEKF_Q(double soc)
{
    if(g_group_state == GROUP_STATE_charging)
    {
        return (1 + (soc/100*MAX_EKF_Q_PERCENT)*(soc/100*MAX_EKF_Q_PERCENT));
    }else if(g_group_state == GROUP_STATE_discharging){
        return (1 + ((100-soc)/100*MAX_EKF_Q_PERCENT)*((100-soc)/100*MAX_EKF_Q_PERCENT));
    }
}





uint32_t getEKF_R(double H)
{

    if(H<1)
    {
        return VOL_SAMPLE_ERR_MV_1*VOL_SAMPLE_ERR_MV_1;
    }else if(H>2)
    {
        return VOL_SAMPLE_ERR_MV_3*VOL_SAMPLE_ERR_MV_3;
    }else{
        int t = fabs(VOL_SAMPLE_ERR_MV_3-VOL_SAMPLE_ERR_MV_1);
        return (VOL_SAMPLE_ERR_MV_1+(H-1)*t)*(VOL_SAMPLE_ERR_MV_1+(H-1)*t);
    }
  
}



void mysocEKF(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra, float soh)
{
    static int callCount = 0;
    callCount++;
    static double pureAHSUM = 0;

    const uint16_t *curve = get_curve_v(cur, tempra);
    const int16_t *curveK = get_curve_k(cur, tempra);
    const uint16_t cap = get_cap(cur, tempra);

    const double capf = cap/10.0*(soh/100);

    double diffAH = DIFF_T_SEC/3600.0*cur/capf*100;
    double SOCcal = SOCinfo->soc + diffAH;
    double Q = getEKF_Q(SOCcal);
    double SOCer2Cal = SOCinfo->socEr2 + Q;
    pureAHSUM += diffAH;
    // printf("diffAH : %f, EKF_W : %f pureAH: %f\n", diffAH, EKF_W(diffAH,capf, cur), pureAHSUM);
    double H = 0, Hprev = 0, Hnext = 0;
    double estVol = 0, estVolPrev = 0, estVolNext = 0;
    double SOCerCal = sqrt(SOCer2Cal);
    if(SOCcal < 0)
    {
        SOCcal = 0;

        estVol = curve[0];
        H = (double)curveK[0]/10;
    }else if(SOCcal > 100)
    {
        SOCcal = 100;

        estVol = curve[SOC_POINT_NUM-1];
        H = (double)curveK[SOC_POINT_NUM-1]/10;
    }else{
        if(SOCcal < 5)
        {
            double sock = SOCcal-(int)SOCcal;
            estVolPrev = curve[(int)SOCcal];
            estVolNext = curve[(int)SOCcal+1];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }else if(SOCcal < 95)
        {
            double sock = ((int)SOCcal%5+SOCcal-(int)SOCcal)/5;
            estVolPrev = curve[(int)SOCcal/5+4];
            estVolNext = curve[(int)SOCcal/5+1+4];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }    
        else{
            double sock = SOCcal-(int)SOCcal;
            estVolPrev = curve[23+(int)SOCcal-95];
            estVolNext = curve[23+(int)SOCcal-95+1];
            estVol = estVolPrev + sock*(estVolNext-estVolPrev);
        }
        // printf("callcount %d soccal : %f  estVolPrev :%f  estVol : %f  estVolNext :%f \n", callCount, SOCcal, estVolPrev, estVol, estVolNext);

        if(SOCcal < 5)
        {
            double Hk = SOCcal-(int)SOCcal;
            Hprev = (double)curveK[((int)SOCcal)]/10;
            Hnext = (double)curveK[(int)SOCcal+1]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }else if(SOCcal < 95)
        {
            double Hk = ((int)SOCcal%5+SOCcal-(int)SOCcal)/5;
            Hprev = (double)curveK[((int)SOCcal/5+4)]/10;
            Hnext = (double)curveK[(int)SOCcal/5+1+4]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }    
        else{
            double Hk = SOCcal-(int)SOCcal;
            Hprev = (double)curveK[23+(int)SOCcal-95]/10;
            Hnext = (double)curveK[23+(int)SOCcal-95+1]/10;
            H = Hprev + Hk*(Hnext-Hprev); 
        }
        // printf("callcount %d hprev :%f  H : %f  hnext :%f \n", callCount, Hprev, H, Hnext);
    }
    double K = 0;
    uint32_t ekfR = getEKF_R(H);

    K = SOCer2Cal*H/(H*SOCer2Cal*H+ekfR);

    
    double res = SOCcal+K*((double)vol-estVol);
    // printf("callcount %d  H: %f K :%f  kcal : %f , vol: %d, estvol: %lf\n", callCount, H, K, K*((double)vol-estVol), vol, estVol);
    double resEr2 = (1-K*H)*SOCer2Cal;
    if(resEr2 > Q)
    {
        resEr2 = Q;
    }
    double SOCerRes = sqrt(resEr2);


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
            res = SOCinfo->soc;
            // resEr2 = SOCinfo->socEr2;
            // printf("soc underflow\n");
        }
    }else if(cur < 0)
    {
        if(res > SOCinfo->soc)
        {
            res = SOCinfo->soc;
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
    //printf("%d soc error2: %f Q:%f R:%d, H:%f, K:%lf kcal : %f , vol: %d, estvol: %lf\n",callCount, SOCinfo->socEr2, Q, ekfR, H, K, K*((double)vol-estVol), vol, estVol);
}






void mysoc(struct SOC_Info *SOCinfo, float cur, uint16_t vol, int16_t tempra, float soh)
{
    if(fabs(cur) > CUR_WINDOW_A)
    {
        if(tempra<0 || (tempra < PURE_AH_LOCK_TEMP_THRESHOLD && fabs(cur) > PURE_AH_LOCK_CUR_THRESHOLD)){
            SOCinfo->pureAH_lock = true;
        }
        if(SOCinfo->pureAH_lock)
        {
            mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);
        }else{
            enum GroupState state = check_current_group_state(cur);
            if(state == GROUP_STATE_transfer){
                mysoc_pureAH(SOCinfo, cur, vol, tempra, soh);               // Ampere-hour Integration only
            }else{
                mysocEKF(SOCinfo, cur, vol, tempra, soh);                   // Ampere-hour Integration + EKF
            }
        }
        mysoc_smooth(SOCinfo, cur, vol, tempra, soh);
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

static void bubbleSort_ascend_duble(double *inputArr, double *outputArr, uint16_t size)
{
    memcpy(outputArr, inputArr, size*sizeof(double));
    for (size_t i = 0; i < size-1; i++)
    {
        for (size_t j = 0; j < size-i-1; j++)
        {
            if(outputArr[j] > outputArr[j+1])
            {
                double tmp = outputArr[j];
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
    static 
    uint16_t sortedSOC[CELL_NUMS];
    bubbleSort_ascend(g_celSOC, sortedSOC, CELL_NUMS);
    uint16_t maxSOC = sortedSOC[CELL_NUMS-1];
    uint16_t minSOC = sortedSOC[0];
    // if(callCount == 3560){
    //     printf("change\n");
    // }
    uint16_t hSOC = maxSOC*(maxSOC/1000.0)+minSOC*(1-maxSOC/1000.0);
    uint16_t lSOC= minSOC*(1-minSOC/1000.0)+maxSOC*(minSOC/1000.0);
    uint16_t h = hSOC>lSOC?hSOC:lSOC;
    uint16_t l = lSOC<hSOC?lSOC:hSOC;
    *g_grpSOC = (*g_grpSOC)/1000.0*h+(1000-*g_grpSOC)/1000.0*l;

    // printf("call: %d, hSOC:%d, lSOC:%d, grpSOC:%d\n",callCount, hSOC, lSOC, *g_grpSOC);

}

/**
 * @brief  convert voltage to soc by lookup table
 * @return soc
 */
static float vol2soc(uint16_t vol, uint16_t tempra)
{
    return 0;
}

static void vol2soc_batch(uint16_t *vol, uint16_t *tempra, float *soc)
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
                else if(fabs(soc_saved[i]-soc_lookuptable[i]) > 50)
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
            *g_grpSOC = round(avg_soc*10);
        }
    }else{
        *g_grpSOC = round(avg_soc*10);
    }



    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        g_socInfo[i].soc = SOC0;
        g_socInfo[i].socEr2 = SOC0_ER2;
        g_celSOC[i] = round(SOC0*10);

    }
    *g_grpSOC = round(SOC0*10);
    
    
}

void soc_save()
{
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
    if(fabs(*g_grpSOC-last_grpsoc) > SOC_GRP_SAVE_DIFF_PERCENT*10)
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

    
}






void soc_task(bool full, bool empty)
{
    for (size_t i = 0; i < CELL_NUMS; i++)
    {

        mysoc(&g_socInfo[i], *g_cur, g_celVol[i], g_celTmp[i], g_celSOH[i]);

        // output soc
        if(g_socInfo[i].soc_smooth && fabs(g_socInfo[i].soc_smooth-g_socInfo[i].soc) > 2)
        {
            g_socInfo[i].soc = g_socInfo[i].soc_smooth*10;
        }else{
            g_celSOC[i] = round(fabs(g_socInfo[i].soc)*10);
        }
        

        // if(i == 0)
        // {
            // printf("soc error2: %f \n", g_socInfo[0].socEr2);
        // }
    }
    if(full)
    {
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            g_socInfo[i].soc = 100;
            g_celSOC[i] = 1000;
        }
    }
    if(empty)
    {
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            g_socInfo[i].soc = 0;
            g_celSOC[i] = 0;
        }
    }
    enum GroupState state = check_current_group_state(*g_cur);
    if(state == GROUP_STATE_standby)
    {
        for (size_t i = 0; i < CELL_NUMS; i++)
        {
            g_socInfo[i].pureAH_lock = false;
            g_socInfo[i].soc_smooth = 0;
            memset(g_socInfo[i].cell_vol_buffer, 0, sizeof(g_socInfo[i].cell_vol_buffer));
            g_socInfo[i].cell_vol_buff_idx = 0;
        }
        
    }
    gropuSOC();


}




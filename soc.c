#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "soc.h"

#define DIFF_T_MSEC                  1           //second
#define CAPACITY_AH                 100         //AH
#define CUR_SAMPLE_ERR_A            1.0         //A
#define VOL_SAMPLE_ERR_MV           10          //mv
#define CUR_WINDOW_A                0.5         //A

#define CELL_NUMS                   1        


#define EKF_Q                       ((CUR_SAMPLE_ERR_A/3600)*(CUR_SAMPLE_ERR_A/3600))     
#define EKF_R                       (VOL_SAMPLE_ERR_MV*VOL_SAMPLE_ERR_MV)

#define TEMP_POINT_NUM              12      // 0 5 10 15 20 25 30 35 40 45 50 55   
#define CUR_POINT_NUM               5       // 0.1 0.2 0.3 0.4 0.5
#define SOC_POINT_STEP              5
#define SOC_POINT_NUM               (100/SOC_POINT_STEP+1)

#define SOC0            40
#define SOC0_ER2        1600

// input 
float *g_cur;
uint16_t *g_celVol;
uint16_t *g_celTmp;
// output
uint16_t *g_celSOC;
uint16_t *g_grpSOC;

struct SOC_Info
{
    float soc;
    float socEr2;
    float soc_last_res;
    bool init;
};
struct SOC_Info g_socInfo[CELL_NUMS];




const uint16_t s_chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {
        3090, 3298, 3319, 3348,         // 0 5 10 15
        3370, 3380, 3382, 3383,         // 20 25 30 35
        3384, 3386, 3390, 3395,         // 40 45 50 55
        3400, 3404, 3407, 3411,         // 60 65 70 75
        3416, 3422, 3431, 3448, 3534},  // 80 85 90 95 100
};

const uint16_t s_chg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {
        1270, 35, 55, 50, 
        35, 10, 2, 5, 
        2, 5, 10, 10, 
        10, 10, 5, 10, 
        10, 15, 25, 50, 400},

};

const uint16_t s_dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {}
};

const uint16_t s_dsg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {}
};



static const uint16_t * get_curve(float cur, uint16_t tempra)
{
    if(cur > 0)
    {
        return s_chg_curve[0][0];
    }else if(cur < 0)
    {
        return s_dsg_curve[0][0];
    }
}

static const uint16_t * get_curve_k(float cur, uint16_t tempra)
{
    if(cur > 0)
    {
        return s_chg_curve_k[0][0];
    }else if(cur < 0)
    {
        return s_dsg_curve_k[0][0];
    }
}





void mysocEKF(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra)
{
    static int callCount = 0;
    callCount++;


    const uint16_t *curve = get_curve(cur, tempra);
    const uint16_t *curveK = get_curve_k(cur, tempra);

    float diffAH = DIFF_T_MSEC/3600.0*cur/CAPACITY_AH*100;
    float SOCcal = SOCinfo->soc + diffAH;
    float SOCer2Cal = SOCinfo->socEr2 + EKF_Q;

    float H = 0, Hprev = 0, Hnext = 0;
    float estVol = 0, estVolPrev = 0, estVolNext = 0;
    float SOCerCal = sqrt(SOCer2Cal);
    if(SOCcal < 0.01)
    {
        SOCcal = 0;

        estVol = curve[0];
        H = (float)curveK[0]/10;
    }else if(SOCcal > 99.99)
    {
        SOCcal = 100;

        estVol = curve[SOC_POINT_NUM-1];
        H = (float)curveK[SOC_POINT_NUM-1]/10;
    }else{
        estVolPrev = curve[(int)SOCcal/SOC_POINT_STEP];
        estVolNext = curve[(int)SOCcal/SOC_POINT_STEP+1];
        estVol = estVolPrev+((int)SOCcal%SOC_POINT_STEP+SOCcal-(int)SOCcal)/SOC_POINT_STEP*(estVolNext-estVolPrev);

        Hprev = (float)curveK[((int)SOCcal)/SOC_POINT_STEP]/10;
        Hnext = (float)curveK[(int)SOCcal/SOC_POINT_STEP+1]/10;
        H = Hprev + ((int)SOCcal%SOC_POINT_STEP+SOCcal-(int)SOCcal)/SOC_POINT_STEP*(Hnext-Hprev);
    }
    float K = SOCer2Cal*H/(H*SOCer2Cal*H+EKF_R);
    float res = SOCcal+K*((float)vol-estVol);
    float resEr2 = (1-K*H)*SOCer2Cal;
    float SOCerRes = sqrt(resEr2);

    if(!SOCinfo->init){
        SOCinfo->soc_last_res = res;
        SOCinfo->init = true;
    }

    float res_k = (res-SOCinfo->soc_last_res);
    SOCinfo->soc_last_res = res;
    

    if(res < 0.01)
    {
        res = 0;
    }else if(res > 99.99)
    {
        res = 100;
    }

    float modkH = 0;
    if(vol < curve[0]){
        modkH = (float)curveK[0]/10;
    }else if(vol > curve[SOC_POINT_NUM-1]){
        modkH = (float)curveK[SOC_POINT_NUM-1]/10;
    }else{
        int i=0;
        for(; i<SOC_POINT_NUM-1; i++)
        {
            if(vol >= curve[i] && vol < curve[i+1])
            {
                break;
            }
        }
        float modkHprev = (float)curveK[i]/10;
        float modkHnext = (float)curveK[i+1]/10;
        float t = 1.0*(vol-curve[i])/(curve[i+1]-curve[i]);
        modkH = modkHprev + t*(modkHnext-modkHprev);
        // printf("%f %f\n", t, modkH);
    }

    if(cur > 0){
        if(res < SOCinfo->soc)
        {
            float modk = 1/(modkH);




            float slowk = SOCinfo->soc - SOCinfo->soc_last_res;

            // modk = 1/slowk;

            if(modk > 1){
                modk = 1;
            }

            res = SOCinfo->soc + modk*diffAH;
            printf("%d %f %f %f %f\n", callCount, modk, slowk, SOCinfo->soc_last_res, SOCerRes);
            // printf("%d %f %f %f %f\n", callCount, modk, modkH, SOCinfo->soc_last_res, SOCerRes);
        }
    }else if(cur < 0)
    {
        if(res > SOCinfo->soc)
        {
            float modk = 1/(modkH);
            if(modk > 1){
                modk = 1;
            }
            res = SOCinfo->soc + modk*diffAH;
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


void mysoc(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra)
{

    if(fabs(cur) > CUR_WINDOW_A)
    {
        mysocEKF(SOCinfo, cur, vol, tempra);

    }else{


    }

}



static void gropuSOC(void)
{







}








void SOC_Init(float *cur, uint16_t *vol, uint16_t *tmp, uint16_t *soc, uint16_t *grpSOC)
{
    g_cur = cur;
    g_celVol = vol;
    g_celTmp = tmp;
    g_celSOC = soc;
    g_grpSOC = grpSOC;
    // todo ocv calibration,set init soc and soc_er2
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        g_socInfo[i].soc = SOC0;
        // g_socInfo[i].soc_last_res = SOC0;
        g_socInfo[i].socEr2 = SOC0_ER2;
        g_celSOC[i] = round(SOC0*10);

    }
    
    
}


void SOC_Task(void)
{
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        mysoc(&g_socInfo[i], *g_cur, g_celVol[i], g_celTmp[i]);
        g_celSOC[i] = round(fabs(g_socInfo[i].soc)*10);
    }
}




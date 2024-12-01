#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "soc.h"

#define EKF_Q                       ((CUR_SAMPLE_ERR_A/CAPACITY_AH)*(CUR_SAMPLE_ERR_A/CAPACITY_AH))     
#define EKF_R                       (VOL_SAMPLE_ERR_MV*VOL_SAMPLE_ERR_MV)

#define TEMP_POINT_NUM              12      // 0 5 10 15 20 25 30 35 40 45 50 55   
#define CUR_POINT_NUM               5       // 0.1 0.2 0.3 0.4 0.5
#define SOC_POINT_NUM               101     // 0  2  4  6....100



// input 
float *g_cur;
uint16_t *g_celVol;
uint16_t *g_celTmp;
// output
uint16_t *g_celSOC;

struct SOC_Info
{
    float soc;
    float socEr2;
};
struct SOC_Info g_socInfo[CELL_NUMS];




const uint16_t s_chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {3090, 3217, 3258, 3284, 3293, 3298, 3301, 3305, 3309, 3314, 3319, 3325, 3331, 3336, 3342, 3348, 3353, 3358, 3362, 3366, 3370, 3373, 3376, 3378, 3379, 3380, 3381, 3381, 3382, 3382, 3382, 3382, 3382, 3382, 3382, 3383, 3383, 3383, 3383, 3384, 3384, 3384, 3385, 3385, 3386, 3386, 3387, 3388, 3388, 3389, 3390, 3391, 3392, 3393, 3394, 3395, 3396, 3397, 3398, 3399, 3400, 3401, 3402, 3402, 3403, 3404, 3405, 3405, 3406, 3407, 3407, 3408, 3409, 3409, 3410, 3411, 3412, 3413, 3414, 3415, 3416, 3417, 3418, 3419, 3420, 3422, 3423, 3425, 3427, 3429, 3431, 3433, 3436, 3439, 3443, 3447, 3453, 3460, 3471, 3487, 3519},
};

const float s_chg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {170.0000, 84.0000, 33.5000, 17.5000,  7.0000,  4.0000,  3.5000,  4.0000,  4.5000,  5.0000,  5.5000,  6.0000,  5.5000,  5.5000,  6.0000,  5.5000,  5.0000,  4.5000,  4.0000,  4.0000,  3.5000,  3.0000,  2.5000,  1.5000,  1.0000,  1.0000,  0.5000,  0.5000,  0.5000,  0.1000,  0.1000,  0.1000,  0.1000,  0.1000,  0.5000,  0.5000,  0.1000,  0.1000,  0.5000,  0.5000,  0.1000,  0.5000,  0.5000,  0.5000,  0.5000,  0.5000,  1.0000,  0.5000,  0.5000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  0.5000,  0.5000,  1.0000,  1.0000,  0.5000,  0.5000,  1.0000,  0.5000,  0.5000,  1.0000,  0.5000,  0.5000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.5000,  1.5000,  1.5000,  2.0000,  2.0000,  2.0000,  2.0000,  2.5000,  3.0000,  3.5000,  4.0000,  5.0000,  6.5000,  9.0000, 13.5000, 24.0000, 40.0000},
};

const uint16_t s_dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {}
};

const float s_dsg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
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

static const float * get_curve_k(float cur, uint16_t tempra)
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
    const uint16_t *curve = get_curve(cur, tempra);
    const float *curveK = get_curve_k(cur, tempra);

    float diffAH = DIFF_T_MSEC/3600.0*cur/CAPACITY_AH*100;
    float SOCcal = SOCinfo->soc + diffAH;
    float SOCer2Cal = SOCinfo->socEr2 + EKF_Q;

    uint16_t H = 0, Hprev = 0, Hnext = 0;
    uint16_t estVol = 0, estVolPrev = 0, estVolNext = 0;
    float SOCerCal = sqrt(SOCer2Cal);
    if(SOCcal < 0)
    {
        float newSOCerCal = fabs(SOCerCal-fabs(SOCcal));
        SOCcal = 0;
        SOCer2Cal = newSOCerCal*newSOCerCal;
    }

    if(SOCcal > 100)
    {
        float newSOCerCal = fabs(SOCerCal-fabs(SOCcal-100));
        SOCcal = 100;
        SOCer2Cal = newSOCerCal*newSOCerCal;

        estVol = curve[SOC_POINT_NUM-1];
        H = curveK[SOC_POINT_NUM-1];
    }else{
        estVolPrev = curve[(int)SOCcal];
        estVolNext = curve[(int)SOCcal+1];
        estVol = estVolPrev+(SOCcal-(int)SOCcal)*(estVolNext-estVolPrev);
        Hprev = curveK[(int)SOCcal];
        Hnext = curveK[(int)SOCcal+1];
        H = Hprev + (SOCcal-(int)SOCcal)*(Hnext-Hprev);
    }
    float K = SOCer2Cal*H/(H*SOCer2Cal*H+EKF_R);
    float res = SOCcal+K*(vol-estVol);
    float resEr2 = (1-K*H)*SOCer2Cal;
    float SOCerRes = sqrt(resEr2);

    if(cur > 0){
        if(res < SOCinfo->soc)
        {
            float modk = 1/(SOCinfo->soc-res);
            if(modk > 1){
                modk = 1;
            }
            printf("4444444 %f\n", modk);

            float tres = res;
            res = SOCinfo->soc + modk*diffAH;
            SOCerRes = fabs(tres-SOCinfo->soc);
            resEr2 = SOCerRes * SOCerRes;
        }
    }else if(cur < 0)
    {
        if(res > SOCinfo->soc)
        {
            float modk = 1/(res-SOCinfo->soc);
            if(modk > 1){
                modk = 1;
            }
            float tres = res;
            res = SOCinfo->soc + modk*diffAH;
            SOCerRes = fabs(tres-SOCinfo->soc);
            resEr2 = SOCerRes * SOCerRes;
        }
    }


    if(res < 0)
    {
        float newSOCerRes = fabs(SOCerRes-fabs(res));
        res = 0;
        resEr2 = newSOCerRes*newSOCerRes;
    }else if(res > 100)
    {
        float newSOCerRes = fabs(SOCerRes-fabs(res-100));
        res = 100;
        resEr2 = newSOCerRes*newSOCerRes;
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




void SOC_Init(float *cur, uint16_t *vol, uint16_t *tmp, uint16_t *soc)
{
    g_cur = cur;
    g_celVol = vol;
    g_celTmp = tmp;
    g_celSOC = soc;
    // todo ocv calibration,set init soc and soc_er2
    for (size_t i = 0; i < CELL_NUMS; i++)
    {
        g_socInfo[i].soc = 10;
        g_socInfo[i].socEr2 = 100;
        soc[i] = 100;
    }
    
    
}


void SOC_Task(void)
{
    for (size_t i = 0; i < 1; i++)
    {
        printf("%f %d\n", *g_cur, g_celVol[i]);

        mysoc(&g_socInfo[i], *g_cur, g_celVol[i], g_celTmp[i]);
        
        printf("3333333 %f \n", g_socInfo[i].soc);

        g_celSOC[i] = fabs(g_socInfo[i].soc)*10;
    }
}




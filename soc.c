#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "soc.h"

#define EKF_Q                       ((CUR_SAMPLE_ERR_A/3600)*(CUR_SAMPLE_ERR_A/3600))     
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
uint16_t *g_grpSOC;

struct SOC_Info
{
    float soc;
    float socEr2;
};
struct SOC_Info g_socInfo[CELL_NUMS];




const uint16_t s_chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {
        3090, 3217, 3258, 3284, 3293, 3298, 3301, 3305, 3309, 3314, 
        3319, 3325, 3331, 3336, 3342, 3348, 3353, 3358, 3362, 3366, 
        3370, 3373, 3376, 3378, 3379, 3380, 3381, 3381, 3382, 3382, 
        3382, 3382, 3382, 3382, 3382, 3383, 3383, 3383, 3383, 3384, 
        3384, 3384, 3385, 3385, 3386, 3386, 3387, 3388, 3388, 3389, 
        3390, 3391, 3392, 3393, 3394, 3395, 3396, 3397, 3398, 3399, 
        3400, 3401, 3402, 3402, 3403, 3404, 3405, 3405, 3406, 3407, 
        3407, 3408, 3409, 3409, 3410, 3411, 3412, 3413, 3414, 3415, 
        3416, 3417, 3418, 3419, 3420, 3422, 3423, 3425, 3427, 3429, 
        3431, 3433, 3436, 3439, 3443, 3447, 3453, 3460, 3471, 3487, 3519},
};

const float s_chg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM][SOC_POINT_NUM] = 
{
    [0][0] = {
    170.0000,   84.0000, 33.5000, 17.5000,  7.0000,  4.0000,  3.5000,  4.0000,  4.5000,  5.0000,  
    5.5000,     6.0000,  5.5000,  5.5000,   6.0000,  5.5000,  5.0000,  4.5000,  4.0000,  4.0000,  
    3.5000,     3.0000,  2.5000,  1.5000,   1.0000,  1.0000,  0.5000,  0.5000,  0.5000,  0.1000,  
    0.1000,     0.1000,  0.1000,  0.1000,   0.5000,  0.5000,  0.1000,  0.1000,  0.5000,  0.5000,  
    0.1000,     0.5000,  0.5000,  0.5000,   0.5000,  0.5000,  1.0000,  0.5000,  0.5000,  1.0000,  
    1.0000,     1.0000,  1.0000,  1.0000,   1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  
    1.0000,     1.0000,  0.5000,  0.5000,   1.0000,  1.0000,  0.5000,  0.5000,  1.0000,  0.5000,  
    0.5000,     1.0000,  0.5000,  0.5000,   1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  1.0000,  
    1.0000,     1.0000,  1.0000,  1.0000,   1.5000,  1.5000,  1.5000,  2.0000,  2.0000,  2.0000,  
    2.0000,     2.5000,  3.0000,  3.5000,   4.0000,  5.0000,  6.5000,  9.0000, 13.5000, 24.0000, 40.0000},
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

    float H = 0, Hprev = 0, Hnext = 0;
    uint16_t estVol = 0, estVolPrev = 0, estVolNext = 0;
    float SOCerCal = sqrt(SOCer2Cal);
    if(SOCcal < 0.01)
    {
        SOCcal = 0;

        estVol = curve[0];
        H = curveK[0];
    }else if(SOCcal > 99.99)
    {
        SOCcal = 100;

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

    if(res < 0.01)
    {
        res = 0;
    }else if(res > 99.99)
    {
        res = 100;
    }

    float modkH = 0;
    if(vol < curve[0]){
        modkH = curveK[0];
    }else if(vol > curve[SOC_POINT_NUM-1]){
        modkH = curveK[SOC_POINT_NUM-1];
    }else{
        int i=0;
        for(; i<SOC_POINT_NUM; i++)
        {
            if(vol >= curve[i] && vol < curve[i+1])
            {
                break;
            }
        }
        float modkHprev = curveK[i];
        float modkHnext = curveK[i];
        modkH = modkHprev + 1.0*(vol-curve[i])/(curve[i+1]-curve[i])*(modkHnext-modkHprev);
    }

    if(cur > 0){
        if(res < SOCinfo->soc)
        {
            float modk = 1/(modkH);
            if(modk > 1){
                modk = 1;
            }
            res = SOCinfo->soc + modk*diffAH;
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





#define SOC0            20
#define SOC0_ER2        400


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
        g_socInfo[i].socEr2 = SOC0_ER2;
        g_celSOC[i] = round(SOC0*10);
    }
    
    
}


void SOC_Task(void)
{
    for (size_t i = 0; i < 1; i++)
    {
        mysoc(&g_socInfo[i], *g_cur, g_celVol[i], g_celTmp[i]);
        g_celSOC[i] = round(fabs(g_socInfo[i].soc)*10);
    }
}




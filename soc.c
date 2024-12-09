#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "soc.h"

#define DIFF_T_MSEC                  1           //second
#define CAPACITY_AH                 100         //AH
#define CAP_ERR_AH                  5           
#define CUR_SAMPLE_ERR_A            1.0         //A
#define VOL_SAMPLE_ERR_MV           10          //mv
#define CUR_WINDOW_A                0.5         //A

#define CELL_NUMS                   16        


#define EKF_Q                       ((CUR_SAMPLE_ERR_A/180)*(CUR_SAMPLE_ERR_A/180))     
#define EKF_R                       (VOL_SAMPLE_ERR_MV*VOL_SAMPLE_ERR_MV)

#define TEMP_POINT_NUM              7      // 0 5  15 25   35  45  55   
#define CUR_POINT_NUM               5       // 0.1 0.2 0.3 0.4 0.5
#define SOC_POINT_STEP              5
#define SOC_POINT_NUM               (100/SOC_POINT_STEP+1)

#define SOC0            50
#define SOC0_ER2        2500

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


const uint16_t vol_dregree25point5[SOC_POINT_NUM] = {        
    3090, 3298, 3319, 3348,         // 0 5 10 15
    3370, 3380, 3382, 3383,         // 20 25 30 35
    3384, 3386, 3390, 3395,         // 40 45 50 55
    3400, 3404, 3407, 3411,         // 60 65 70 75
    3416, 3422, 3431, 3448, 3534    // 80 85 90 95 100
};
const uint16_t k_dregree25point5[SOC_POINT_NUM] = {        
    1270,   35,     55,     50,                     // 0 5 10 15
    35,     10,     2,      5,                      // 20 25 30 35
    2,      5,      10,     10,                     // 40 45 50 55
    10,     10,     5,      10,                     // 60 65 70 75
    10,     15,     25,     50,     400             // 80 85 90 95 100
};








const uint16_t* s_chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM]= 
{
    /*  0.1C             0.2 C                0.3 C                0.4 C                    0.5C*/
    [0][0] = NULL,      [0][1] = NULL,      [0][2] = NULL,      [0][3] = NULL,      [0][4] = NULL,                          // 0
    [1][0] = NULL,      [1][1] = NULL,      [1][2] = NULL,      [1][3] = NULL,      [1][4] = NULL,                          //5
    [2][0] = NULL,      [2][1] = NULL,      [2][2] = NULL,      [2][3] = NULL,      [2][4] = NULL,                          //15
    [3][0] = NULL,      [3][1] = NULL,      [3][2] = NULL,      [3][3] = NULL,      [3][4] = vol_dregree25point5,           //25    
    [4][0] = NULL,      [4][1] = NULL,      [4][2] = NULL,      [4][3] = NULL,      [4][4] = NULL,                          //35
    [5][0] = NULL,      [5][1] = NULL,      [5][2] = NULL,      [5][3] = NULL,      [5][4] = NULL,                          //45
    [6][0] = NULL,      [6][1] = NULL,      [6][2] = NULL,      [6][3] = NULL,      [6][4] = NULL,                          //55
};

const uint16_t* s_chg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM] = 
{
        /*  0.1C             0.2 C                0.3 C                0.4 C                    0.5C*/
    [0][0] = NULL,      [0][1] = NULL,      [0][2] = NULL,      [0][3] = NULL,      [0][4] = NULL,                          // 0
    [1][0] = NULL,      [1][1] = NULL,      [1][2] = NULL,      [1][3] = NULL,      [1][4] = NULL,                          //5
    [2][0] = NULL,      [2][1] = NULL,      [2][2] = NULL,      [2][3] = NULL,      [2][4] = NULL,                          //15
    [3][0] = NULL,      [3][1] = NULL,      [3][2] = NULL,      [3][3] = NULL,      [3][4] = k_dregree25point5,             //25    
    [4][0] = NULL,      [4][1] = NULL,      [4][2] = NULL,      [4][3] = NULL,      [4][4] = NULL,                          //35
    [5][0] = NULL,      [5][1] = NULL,      [5][2] = NULL,      [5][3] = NULL,      [5][4] = NULL,                          //45
    [6][0] = NULL,      [6][1] = NULL,      [6][2] = NULL,      [6][3] = NULL,      [6][4] = NULL,                          //55

};

const uint16_t* s_dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM] = 
{
    [0][0] = NULL,
};

const uint16_t* s_dsg_curve_k[TEMP_POINT_NUM][CUR_POINT_NUM] = 
{
    [0][0] = NULL,
};

static const uint16_t * get_k_or_v(const uint16_t *chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], const uint16_t *dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], float cur, uint16_t tempra)
{
    if(cur > 0)
    {
        float t = tempra/10.0;
        int tidx = 0;
        if(t<2.5)
        {
            tidx = 0;
        }
        else if(t<10)
        {
            tidx = 2;
        }
        else if(t<50)
        {
            tidx = (int)t/10+1;
        }else{
            tidx = 6;
        }
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

        return chg_curve[tidx][cidx];

    }else if(cur < 0)
    {
        return dsg_curve[0][0];
    }
}

static const uint16_t * get_curve_v(float cur, uint16_t tempra)
{
    return get_k_or_v(s_chg_curve, s_dsg_curve, cur, tempra);
}

static const uint16_t * get_curve_k(float cur, uint16_t tempra)
{
    return get_k_or_v(s_chg_curve_k, s_dsg_curve_k, cur, tempra);
}





void mysocEKF(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra)
{
    static int callCount = 0;
    callCount++;


    const uint16_t *curve = get_curve_v(cur, 250);
    const uint16_t *curveK = get_curve_k(cur, 250);

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


    if(res < 0.01)
    {
        res = 0;
    }else if(res > 99.99)
    {
        res = 100;
    }



    if(cur > 0){
        if(res < SOCinfo->soc)
        {
            res = SOCinfo->soc;
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
}


void mysoc(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra)
{

    if(fabs(cur) > CUR_WINDOW_A)
    {
        mysocEKF(SOCinfo, cur, vol, tempra);

    }else{


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

static void gropuSOC()
{
    uint16_t sortedSOC[CELL_NUMS];
    bubbleSort_ascend(g_celSOC, sortedSOC, CELL_NUMS);
    uint16_t maxSOC = sortedSOC[CELL_NUMS-1];
    uint16_t minSOC = sortedSOC[0];

    uint16_t hSOC = maxSOC*(maxSOC/1000.0)+minSOC*(1-maxSOC/1000.0);
    uint16_t lSOC= minSOC*(minSOC/1000.0)+maxSOC*(1-minSOC/1000.0);
    *g_grpSOC = (*g_grpSOC)/1000.0*hSOC+(1000-*g_grpSOC)/1000.0*lSOC;
    printf("hSOC:%d, lSOC:%d, grpSOC:%d\n", hSOC, lSOC, *g_grpSOC);

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
        g_socInfo[i].socEr2 = SOC0_ER2;
        g_celSOC[i] = round(SOC0*10);

    }
    *g_grpSOC = round(SOC0*10);
    
    
}


void SOC_Task(bool full, bool empty)
{
    for (size_t i = 0; i < CELL_NUMS; i++)
    {

        mysoc(&g_socInfo[i], *g_cur, g_celVol[i], g_celTmp[i]);

        g_celSOC[i] = round(fabs(g_socInfo[i].soc)*10);
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
    gropuSOC();


}




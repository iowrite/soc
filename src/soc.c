#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "soc.h"
#include "curve.h"

#define CAPACITY_AH                     100             //AH
#define DIFF_T_SEC                      1               //second

#define DIFF_T_MSEC_ERR                 50              // ms
#define CAP_ERR_AH                      5.0           
#define CUR_SAMPLE_ERR_A                1.0             //A
#define VOL_SAMPLE_ERR_MV               10              //mv

#define CUR_WINDOW_A                    0.5             //A

#define CELL_NUMS                   16    


#define EKF_W(diffAH, cap, cur)                 (((1+50.0/(DIFF_T_SEC*1000)) * (1+1/cur) *(1+ CAP_ERR_AH/cap) - 1) *  diffAH)      
#define EKF_Q(diffAH, cap, cur)                 (EKF_W(diffAH, cap, cur)*EKF_W(diffAH, cap, cur))                 
#define EKF_R                                   (VOL_SAMPLE_ERR_MV*VOL_SAMPLE_ERR_MV)



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






static const uint16_t get_cap(float cur, uint16_t tempra)
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
        // if(s_cap_list_chg[tidx][cidx] == 1000){
        //     assert(0);
        // }
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
        if(s_cap_list_dsg[tidx][cidx] == 1000){
            assert(0);
        }
        return s_cap_list_dsg[tidx][cidx];
    }




}




static const uint16_t * get_v(const uint16_t *chg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], const uint16_t *dsg_curve[TEMP_POINT_NUM][CUR_POINT_NUM], float cur, uint16_t tempra)
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





void mysocEKF(struct SOC_Info *SOCinfo, float cur, uint16_t vol, uint16_t tempra)
{
    static int callCount = 0;
    callCount++;
    static float pureAHSUM = 0;

    const uint16_t *curve = get_curve_v(cur, 250);
    const int16_t *curveK = get_curve_k(cur, 250);
    const uint16_t cap = get_cap(cur, 250);

    const float capf = cap/10.0;
    float diffAH = DIFF_T_SEC/3600.0*cur/capf*100;
    float SOCcal = SOCinfo->soc + diffAH;
    float SOCer2Cal = SOCinfo->socEr2 + EKF_Q(diffAH,capf, cur);
    pureAHSUM += diffAH;
    printf("diffAH : %f, EKF_W : %f pureAH: %f\n", diffAH, EKF_W(diffAH,capf, cur), pureAHSUM);
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
        estVolPrev = curve[(int)SOCcal/SOC_POINT_STEP];
        estVolNext = curve[(int)SOCcal/SOC_POINT_STEP+1];
        estVol = estVolPrev+((int)SOCcal%SOC_POINT_STEP+SOCcal-(int)SOCcal)/SOC_POINT_STEP*(estVolNext-estVolPrev);

        Hprev = (float)curveK[((int)SOCcal)/SOC_POINT_STEP]/10;
        Hnext = (float)curveK[(int)SOCcal/SOC_POINT_STEP+1]/10;
        float sock = ((int)SOCcal%SOC_POINT_STEP+SOCcal-(int)SOCcal)/SOC_POINT_STEP;
        if(SOCcal < 5)
        {
            float sock3 = sock*sock*sock;
            H = Hprev + sock3*(Hnext-Hprev);       
        }else if(SOCcal < 95)
        {
            H = Hprev + sock*(Hnext-Hprev);
        }    
        else{
            float sock3 = sock*sock*sock;
            H = Hprev + (1-sock3)*(Hnext-Hprev);

        }
        // printf("callcount %d hprev :%f  H : %f  hnext :%f \n", callCount, Hprev, H, Hnext);
    }
    float K = SOCer2Cal*H/(H*SOCer2Cal*H+EKF_R);
    float res = SOCcal+K*((float)vol-estVol);
    printf("callcount %d  H: %f K :%f  kcal : %f \n", callCount, H, K, K*((float)vol-estVol));
    float resEr2 = (1-K*H)*SOCer2Cal;
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
    printf("soc : %f \n", res);
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
    static int callCount = 0;
    callCount++;
    uint16_t sortedSOC[CELL_NUMS];
    bubbleSort_ascend(g_celSOC, sortedSOC, CELL_NUMS);
    uint16_t maxSOC = sortedSOC[CELL_NUMS-1];
    uint16_t minSOC = sortedSOC[0];
    // if(callCount == 3560){
    //     printf("change\n");
    // }
    uint16_t hSOC = maxSOC*(maxSOC/1000.0)+minSOC*(1-maxSOC/1000.0);
    uint16_t lSOC= minSOC*(minSOC/1000.0)+maxSOC*(1-minSOC/1000.0);
    uint16_t h = hSOC>lSOC?hSOC:lSOC;
    uint16_t l = lSOC<hSOC?lSOC:hSOC;
    *g_grpSOC = (*g_grpSOC)/1000.0*h+(1000-*g_grpSOC)/1000.0*l;
    // printf("call: %d, hSOC:%d, lSOC:%d, grpSOC:%d\n",callCount, hSOC, lSOC, *g_grpSOC);

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




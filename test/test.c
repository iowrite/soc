#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "sox.h"
#include "test.h"

#define FIFO_NAME "myfifo"
#define ARRAY_SIZE 5

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



uint32_t excel_second = 0;
int g_port_init_soc;

int main(int argc, char *argv[]) {

    if(argc < 2)
    {
        printf("Usage: ./test <init_soc>\n");
        return 0;
    }
    g_port_init_soc = atoi(argv[1]);

/* ====================================== input data row len     ================================================= */

    int fd1 = open("socfifo_write_input_row_len", O_RDONLY);
    int input_row_len = 0;
    read(fd1, &input_row_len, sizeof(input_row_len)) != sizeof(input_row_len);
    close(fd1);
    unlink("socfifo_write_input_row_len");
    printf("recv: input_row_len = %d\n", input_row_len);
    
    
/* ======================================= input data================================================ */
    sleep(1);

    int fd2 = open("socfifo_write_input", O_RDONLY);
    struct input{
        float grpVol;
        float cur;
        float avgTmp;
        float vol[16];
        float tmp[9];
    };

    struct input *inputData = malloc(sizeof (struct input) * input_row_len);
    memset(inputData, 0, sizeof(struct input)*input_row_len);

    uint8_t *b = (uint8_t *)inputData;
    int count = sizeof (struct input) * input_row_len;
    printf("count = %d\n", count);
    int r = 0;
    while(count>0)
    {
        r = read(fd2, b, 65536);
        count -= r;
        b += r;
    }
    close(fd2);
    unlink("socfifo_write_input");
    printf("recv: inputData. total bytes = %ld\n", sizeof(struct input)*input_row_len);



    for (size_t i = 0; i < input_row_len; i++)
    {
        // printf("inputData[%d].cur = %f\n", i, inputData[i].cur);
        // if(inputData[i].cur < 0)
        // {
        //     assert(0);
        // }
    }
    

    /************************************* compute ************************************************ */


    struct SOC_Info
    {
        uint16_t soc[16];
    };
    struct GrpSOC
    {
        uint16_t grpSOC;
        uint16_t maxcelsoc;
        uint16_t mincelsoc;
        uint16_t avgcelsoc;
    };

    struct SOC_Info *outputData = malloc(sizeof(struct SOC_Info)*(input_row_len+1));
    struct GrpSOC *outputDataGrp = malloc(sizeof(struct GrpSOC)*(input_row_len+1));

    // struct SOC_Info info = {
    //     .soc = 20,
    //     .socEr2 = 400
    // };
    // memcpy(&outputData[0], &info, sizeof (struct SOC_Info));
    // for(size_t i = 0; i <  input_row_len; i++)
    // {

    //     mysoc(&info, inputData[i].cur, (uint16_t)inputData[i].vol[0], 250);
    //     memcpy(&outputData[i+1], &info, sizeof (struct SOC_Info));
    // }

    float cur;
    uint16_t vol[16];  
    int16_t tmp[16];
    // for(size_t i = 0; i < 16; i++)
    // {
    //     tmp[i] = 250;
    // }
    uint16_t soc[16];
    float soh[16];
    float grpsoh = 100;
    uint32_t cycleCount = 0;
    float grpVol = 0;
    float sigChgWH = 0;
    float accChgAH = 0;
    float sigDsgWH = 0;
    float accDsgAH = 0;
    uint16_t g_chg_stop_vol = 3600;
    uint16_t g_dsg_stop_vol = 2900;
    for (size_t i = 0; i < 16; i++)
    {
        soh[i] = 100;
    }
    
    struct GrpSOC grpSOC;

    memset(soc, 0, sizeof soc);

    for (size_t j = 0; j < 16; j++)
    {
        vol[j] = inputData[0].vol[j];
        //tmp[j] = inputData[i].avgTmp*10;
        tmp[j] = inputData[0].tmp[j]*10;
    }

    sox_init(&cur, vol, tmp, soc, &grpSOC.grpSOC, soh, &grpsoh, &cycleCount, &grpVol, &sigChgWH, &sigDsgWH, &accChgAH, &accDsgAH, &g_chg_stop_vol, &g_dsg_stop_vol);
    memcpy(outputData[0].soc, soc, 32);
    memcpy(outputDataGrp, &grpSOC.grpSOC, 2);
    uint16_t soc_sorted[16];
    bubbleSort_ascend(soc, soc_sorted, 16);
    outputDataGrp[0].maxcelsoc = soc_sorted[15];
    outputDataGrp[0].mincelsoc = soc_sorted[0];

    for (size_t i = 0; i < 16; i++)
    {
        outputDataGrp[0].avgcelsoc += soc[i];
    }
    outputDataGrp[0].avgcelsoc /= 16;
    


    for(size_t i = 0; i < input_row_len; i++)
    {
        excel_second++;
        grpVol = inputData[i].grpVol;
        // printf("inputData[%d].grpVol = %f\n", i, grpVol);
        cur = inputData[i].cur;
        for (size_t j = 0; j < 16; j++)
        {
            vol[j] = inputData[i].vol[j];
            //tmp[j] = inputData[i].avgTmp*10;
        }

        tmp[0]  = inputData[i].tmp[0]*10;
        for(int j = 0; j < 7; j++)
        {
            tmp[j*2+1] = inputData[i].tmp[j+1]*10;
            tmp[j*2+2] = inputData[i].tmp[j+2]*10;
        }
        tmp[15] = inputData[i].tmp[8]*10;

        sox_task(0, 0);

        memcpy(outputData[i+1].soc, soc, 32);
        memcpy(&outputDataGrp[i+1], &grpSOC.grpSOC, 2);
        bubbleSort_ascend(soc, soc_sorted, 16);
        outputDataGrp[i+1].maxcelsoc = soc_sorted[15];
        outputDataGrp[i+1].mincelsoc = soc_sorted[0];
        for (size_t k = 0; k < 16; k++)
        {
            outputDataGrp[i+1].avgcelsoc += soc[k];
        }
        outputDataGrp[i+1].avgcelsoc /= 16;
    }








    /************************************* output  cell soc************************************************ */
    int fd3;

    // Create the FIFO if it doesn't exist
    mkfifo("socfifo_output", 0666);

    // Open the FIFO for writing
    fd3 = open("socfifo_output", O_WRONLY);
    if (fd3 == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write the array to the FIFO
    if (write(fd3, outputData, sizeof(struct SOC_Info)*(input_row_len+1)) != sizeof(struct SOC_Info)*(input_row_len+1)) {
        perror("write");
        close(fd3);
        unlink("socfifo_output");  // Remove the FIFO file
        exit(EXIT_FAILURE);
    }

    // Close the FIFO and remove the FIFO file
    close(fd3);
    unlink("socfifo_output");

    printf("cell soc Array sent successfully\n");

    /************************************* output  grp soc************************************************ */
    int fd4;

    // Create the FIFO if it doesn't exist
    mkfifo("grpsocfifo_output", 0666);

    // Open the FIFO for writing
    fd4 = open("grpsocfifo_output", O_WRONLY);
    if (fd4 == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write the array to the FIFO
    if (write(fd4, outputDataGrp, sizeof( struct GrpSOC)*(input_row_len+1)) != sizeof(struct GrpSOC)*(input_row_len+1)) {
        perror("write");
        close(fd4);
        unlink("grpsocfifo_output");  // Remove the FIFO file
        exit(EXIT_FAILURE);
    }

    // Close the FIFO and remove the FIFO file
    close(fd4);
    unlink("grpsocfifo_output");

    printf("grp soc Array sent successfully\n");











    return 0;
}



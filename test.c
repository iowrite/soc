#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "soc.h"

#define FIFO_NAME "myfifo"
#define ARRAY_SIZE 5






int main() {



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
        float cur;
        float vol[16];
    };

    struct input *inputData = malloc(sizeof (struct input) * input_row_len);
    memset(inputData, 0, sizeof(struct input)*input_row_len);

    uint8_t *b = (uint8_t *)inputData;
    int count = sizeof (struct input) * input_row_len;
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


    /************************************* compute ************************************************ */


    struct SOC_Info
    {
        uint16_t soc[16];
    };

    struct SOC_Info *outputData = malloc(sizeof(struct SOC_Info)*(input_row_len+1));


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
    uint16_t tmp[16];
    uint16_t soc[16];

    memset(soc, 0, sizeof soc);

    SOC_Init(&cur, vol, tmp, soc);
    memcpy(outputData[0].soc, soc, 32);

    for(size_t i = 0; i < input_row_len; i++)
    {
        cur = inputData[i].cur;


        for (size_t j = 0; j < 16; j++)
        {
            vol[j] = inputData[i].vol[j];
        }

        SOC_Task();
        printf("2222222222%d\n", soc[0]);

        memcpy(outputData[i+1].soc, soc, 32);



    }








    /************************************* output  ************************************************ */
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

    printf("Array sent successfully\n");



    return 0;
}



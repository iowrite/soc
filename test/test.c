#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <xlsxio_read.h>
#include "sox.h"
#include <ctype.h>

uint32_t g_excel_second = 0;
extern int g_port_init_soc;


float    s_init_grpsoh = 100;
uint32_t s_init_cycleCount = 0;


uint16_t s_chg_stop_vol = 3600;
uint16_t s_dsg_stop_vol = 2900;




int main(int argc, char *argv[]) 
{
    char whole_cmd[200] = {0};
    for (int i = 0; i < argc; i++) {
        strcat(whole_cmd, argv[i]);
        strcat(whole_cmd, " ");
    }
    printf("%s\n", whole_cmd);
/******************************************************************************
 * parse command line arguments
 ******************************************************************************/
    char input_file_name[100] = {0};
    char opt;
    while((opt = getopt(argc, argv, "hi:s:")) != -1) {
        switch(opt) {
            case 'h':
                printf("./mysoc -h for help information\n");
                printf("./mysoc -i [excel input file] -s [init group soc value]\n");
                return 0;
            case 'i':
                for(unsigned int i = 0; i < strlen(optarg); i++){               // filter leading empty char
                    if(isspace(optarg[i]))
                    {
                        continue;
                    }else{
                        memcpy(input_file_name, optarg+i, strlen(optarg)-i);
                        break;
                    }
                }
                break;
            case 's':
                g_port_init_soc = atoi(optarg);
                break;
            case '?':
                printf("invalid option: %c\n", optopt);
                break;
            case ':':
                printf("option <%c> need argument\n", optopt);
                break;
        }
    }



/*******************************************************************************
 * count total row count
 *******************************************************************************/
    xlsxioreadersheet sheet;
    const char* sheetname = NULL;
    //open .xlsx file for reading
    xlsxioreader xlsxioread;
    if ((xlsxioread = xlsxioread_open(input_file_name)) == NULL) {
        fprintf(stderr, "Error opening .xlsx file\n");
        return 1;
    }
    int s_excel_row = 0;
    // get total row count
    if ((sheet = xlsxioread_sheet_open(xlsxioread, sheetname, XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
        while (xlsxioread_sheet_next_row(sheet)) {
            s_excel_row++;
        }
        xlsxioread_sheet_close(sheet);
    }
    //clean up
    xlsxioread_close(xlsxioread);



/*******************************************************************************
 * parse excel input data 
 *******************************************************************************/
    // reopen .xlsx file for reading content, read values from first sheet
    struct SOX_Excel_Input{
        float grpVol;
        float cur;
        float groupSoc;
        int vol[16];
        float tmp[9];
    };
    struct SOX_Excel_Input *sox_excel_input = malloc(sizeof(struct SOX_Excel_Input) * s_excel_row);
    if ((xlsxioread = xlsxioread_open(input_file_name)) == NULL) {
        fprintf(stderr, "Error opening .xlsx file\n");
        return 1;
    }
    s_excel_row = 0;
    if ((sheet = xlsxioread_sheet_open(xlsxioread, sheetname, XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
        //read all rows
        while (xlsxioread_sheet_next_row(sheet)) {
            int excel_col = 0;
            if(s_excel_row >= 1) //filter the first line(header)
            {
                //read all columns
                double cell_value;
                while (xlsxioread_sheet_next_cell_float(sheet, &cell_value)) {
                    if(excel_col == 9){                                         // group voltage
                        sox_excel_input[s_excel_row-1].grpVol = cell_value;
                    }
                    else if(excel_col == 11){                                   // group current    
                        sox_excel_input[s_excel_row-1].cur = cell_value;
                    }   
                    else if(excel_col == 12){                                   // group soc(microcontroller calculated)
                        sox_excel_input[s_excel_row-1].groupSoc = cell_value;
                    }
                    else if(excel_col >= 76 && excel_col < 92){                 // cell voltage
                        sox_excel_input[s_excel_row-1].vol[excel_col-76] = (int)cell_value;
                    }
                    else if(excel_col >= 108 && excel_col < 117){               // cell temperature
                        sox_excel_input[s_excel_row-1].tmp[excel_col-108] = cell_value;
                    }
                    excel_col++;
                }
            }
            s_excel_row++;
        }
        xlsxioread_sheet_close(sheet);
    }
    //clean up
    xlsxioread_close(xlsxioread);

    

/*******************************************************************************
 * compute 
 *******************************************************************************/
    struct Cell_SOC_Output
    {
        float soc[16];
    };
    struct Group_SOC_Output
    {
        float grpSOC;
    };

    struct Cell_SOC_Output *cell_soc_output = malloc(sizeof(struct Cell_SOC_Output)*(s_excel_row-1)); // exclude header
    struct Group_SOC_Output *group_soc_output = malloc(sizeof(struct Group_SOC_Output)*s_excel_row);  // exclude header but add init value

    // init sox
    struct SOX_Init_Attr attr = {
        .chg_stop_vol = &s_chg_stop_vol,
        .dsg_stop_vol = &s_dsg_stop_vol,
    };

    sox_init(&attr);
    // get init output data
    get_cell_soc_ary(cell_soc_output[0].soc);
    group_soc_output[0].grpSOC = get_group_soc();


    
    for(int i = 0; i < s_excel_row; i++)
    {
        // prepare input data
        struct SOX_Input input = {
            .cur = sox_excel_input[i].cur,
            .grpVol = sox_excel_input[i].grpVol,
            .full = false,
            .empty = false,
        };
        for (int j = 0; j < 16; j++) {
            input.vol[j] = sox_excel_input[i].vol[j];
        }
        // temp 9 to 16
        input.tmp[0] = roundf(sox_excel_input[i].tmp[0]*10);
        for (size_t j = 0; j < 7; j++)
        {
            input.tmp[j*2+1] = roundf(sox_excel_input[i].tmp[j+1]*10);
            input.tmp[j*2+2] = roundf(sox_excel_input[i].tmp[j+2]*10);
        }
        input.tmp[15] = roundf(sox_excel_input[i].tmp[8]*10);

        // caculate
        sox_task(&input);
        // output
        get_cell_soc_ary(cell_soc_output[i+1].soc);
        group_soc_output[i+1].grpSOC = get_group_soc();

        g_excel_second++;
    }





/******************************************************************************
 * write output data to file
 ******************************************************************************/
    // cell soc:simulate calculate
    FILE *output_cell_soc_simulate_fd;

    // Open the file for writing(always overwrite)
    output_cell_soc_simulate_fd = fopen("output_cell_soc_simulate.csv", "w");

    // Write the array to the file
    for (int i = 0; i < s_excel_row; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            fprintf(output_cell_soc_simulate_fd, "%f, ", cell_soc_output[i].soc[j]);
        }
        fprintf(output_cell_soc_simulate_fd, "\n");
    }
    fclose(output_cell_soc_simulate_fd);



    // cell soc: mcu calculate
    // todo: no mcu soc data to export currently



    // group soc(microcontroler calculate)
    FILE *output_group_soc_mcu_fd;

    // Open the file for writing(always overwrite)
    output_group_soc_mcu_fd = fopen("output_group_soc_mcu.csv", "w");

    // Write the array to the file
    for (int i = 0; i < s_excel_row-1; i++)
    {
        fprintf(output_group_soc_mcu_fd, "%f ", sox_excel_input[i].groupSoc);
        fprintf(output_group_soc_mcu_fd, "\n");
    }
    fclose(output_group_soc_mcu_fd);



    // group soc(simulate calculate)
    FILE *output_group_soc_simulate_fd;

    // Open the file for writing(always overwrite)
    output_group_soc_simulate_fd = fopen("output_group_soc_simulate.csv", "w");

    // Write the array to the file
    for (int i = 0; i < s_excel_row; i++)
    {
        fprintf(output_group_soc_simulate_fd, "%f ", group_soc_output[i].grpSOC);
        fprintf(output_group_soc_simulate_fd, "\n");
    }
    fclose(output_group_soc_simulate_fd);





/******************************************************************************
 * call python to draw the result
 ******************************************************************************/
    char call_python_cmd[200] = {0};
    strcat(call_python_cmd, "fish -c \"source /home/hm/Desktop/mysoc/.venv/bin/activate.fish && python3 /home/hm/Desktop/mysoc/test/soc.py ");      // absolute path, fix next version
    strcat(call_python_cmd,"'");
    strcat(call_python_cmd, whole_cmd);
    strcat(call_python_cmd,"' \"");
    printf("%s\n", call_python_cmd);
    system(call_python_cmd);


    return 0;
}



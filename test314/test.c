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
#include "sox_config.h"
#include <ctype.h>

uint32_t g_excel_second = 0;
extern int g_port_init_soc;


float    s_init_grpsoh = 100;
uint32_t s_init_cycleCount = 0;


uint16_t s_chg_stop_vol = 3600;
uint16_t s_dsg_stop_vol = 2900;


static bool compare_pure_AH_SOC = false;


#if CFG_SOX_PORT_SIM_PROJECT == 1
static void project_stack_cell_tempature_map(int16_t *input, float *output);
#elif CFG_SOX_PORT_SIM_PROJECT == 2
static void project_jiguang_cell_tempature_map(int16_t *input, float *output);
#elif CFG_SOX_PORT_SIM_PROJECT == 3
static void cell314_tempature_map(int16_t *input, float *output);
#endif



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
    while((opt = getopt(argc, argv, "hi:s:c")) != -1) {                         
        switch(opt) {
            case 'h':   // h for help
                printf("./mysoc -h for help information\n");
                printf("./mysoc -i [excel input file] -s [init group soc value]\n");
                return 0;
            case 'i':   // i for input file
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
            case 's':   // s for set init soc
                g_port_init_soc = atoi(optarg);
                break;
            case 'c':   // c for compare pure AH soc
                compare_pure_AH_SOC = true;
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

    //open .xlsx file for reading
    xlsxioreader xlsxioread;
    if ((xlsxioread = xlsxioread_open(input_file_name)) == NULL) {
        fprintf(stderr, "Error opening .xlsx file\n");
        return 1;
    }
    int s_excel_row = 0;
    int sheet1_row = 0;
    int sheet2_row = 0;
    int sheet3_row = 0;
    int sheet4_row = 0;
    // get total row count
    if ((sheet = xlsxioread_sheet_open(xlsxioread, "Sheet1", XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
        while (xlsxioread_sheet_next_row(sheet)) {
            sheet1_row++;
        }
        xlsxioread_sheet_close(sheet);
    }

    if ((sheet = xlsxioread_sheet_open(xlsxioread, "Sheet2", XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
        while (xlsxioread_sheet_next_row(sheet)) {
            sheet2_row++;
        }
        xlsxioread_sheet_close(sheet);
    }

    if ((sheet = xlsxioread_sheet_open(xlsxioread, "Sheet3", XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
        while (xlsxioread_sheet_next_row(sheet)) {
            sheet3_row++;
        }
        xlsxioread_sheet_close(sheet);
    }

    if ((sheet = xlsxioread_sheet_open(xlsxioread, "Sheet4", XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
        while (xlsxioread_sheet_next_row(sheet)) {
            sheet4_row++;
        }
        xlsxioread_sheet_close(sheet);
    }
    int min1, min2;
    if(sheet1_row < sheet2_row)
    {
        min1 = sheet1_row;
    }
    else
    {
        min1 = sheet2_row;
    }
    if(sheet3_row < sheet4_row)
    {
        min2 = sheet3_row;
    }
    else
    {
        min2 = sheet4_row;
    }

    s_excel_row = min1 < min2 ? min1 : min2;
    // char *sheetname;
    // if(s_excel_row == sheet1_row){
    //     sheetname = "Sheet1";
    // }else if(s_excel_row == sheet2_row)
    // {
    //     sheetname = "Sheet2";
    // }else if(s_excel_row == sheet3_row)
    // {
    //     sheetname = "Sheet3";     
    // }else{
    //     sheetname = "Sheet4";
    // }
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
        int vol[CELL_NUMS];
        float tmp[CELL_NUMS];
    };
    struct SOX_Excel_Input *sox_excel_input = malloc(sizeof(struct SOX_Excel_Input) * s_excel_row);
    if ((xlsxioread = xlsxioread_open(input_file_name)) == NULL) {
        fprintf(stderr, "Error opening .xlsx file\n");
        return 1;
    }

    for(int i = 0; i < 4; i++)
    {
        char sheetname[10] = {0};
        sprintf(sheetname, "Sheet%d", i+1);
        if ((sheet = xlsxioread_sheet_open(xlsxioread, sheetname, XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL) {
            int j = 0;
            //read all rows
            while (xlsxioread_sheet_next_row(sheet) && j < s_excel_row) {
                int excel_col = 0;
                //read all columns
                double cell_value;
                while (xlsxioread_sheet_next_cell_float(sheet, &cell_value)) {
                    if(excel_col == 7-1){                 // cell voltage
                        sox_excel_input[j].vol[i] = round(cell_value*1000);
                    }
                    else if(excel_col == 8-1){                                   // group current    
                        sox_excel_input[j].cur = cell_value;
                    }   
                    // else if(excel_col == 12){                                   // group soc(microcontroller calculated)
                    //     sox_excel_input[s_excel_row-1].groupSoc = cell_value;
                    // }
                    else if(excel_col == 24){               // cell temperature
                        sox_excel_input[j].tmp[i] = cell_value;
                    }
                    excel_col++;
                }

                j++;
            }
            xlsxioread_sheet_close(sheet);
        }

    }

    //clean up
    xlsxioread_close(xlsxioread);

    

/*******************************************************************************
 * compute 
 *******************************************************************************/
    struct Cell_SOC_Output
    {
        float soc[CELL_NUMS];
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

     struct SOX_Input input0 = {
        .cur = sox_excel_input[0].cur,
        .grpVol = sox_excel_input[0].grpVol,
        .full = false,
        .empty = false,
    };
    int min_vol = sox_excel_input[0].vol[0];
    int max_vol = sox_excel_input[0].vol[0];
    for (int j = 0; j < CELL_NUMS; j++) {
        input0.vol[j] = sox_excel_input[0].vol[j];
        if(sox_excel_input[0].vol[j] < min_vol){
            min_vol = sox_excel_input[0].vol[j];
        }
        if(sox_excel_input[0].vol[j] > max_vol){
            max_vol = sox_excel_input[0].vol[j];
        }
    }
    if(max_vol > s_chg_stop_vol)
    {
        input0.full = true;
    }
    if(min_vol < s_dsg_stop_vol)
    {
        input0.empty = true;
    }
            // asign temp input(map input)
#if CFG_SOX_PORT_SIM_PROJECT == 1
    project_stack_cell_tempature_map(input0.tmp, sox_excel_input[0].tmp);
#elif CFG_SOX_PORT_SIM_PROJECT == 2
    project_jiguang_cell_tempature_map(input0.tmp, sox_excel_input[0].tmp);
#elif CFG_SOX_PORT_SIM_PROJECT == 3
    cell314_tempature_map(input0.tmp, sox_excel_input[0].tmp);
#endif

    sox_init(&attr, &input0);
    // get init output data
    get_cell_soc_ary(cell_soc_output[0].soc);
    group_soc_output[0].grpSOC = get_group_soc();


    
    for(int i = 0; i < s_excel_row-1; i++)
    {

        // prepare input data
        struct SOX_Input input = {
            .cur = sox_excel_input[i].cur,
            .grpVol = sox_excel_input[i].grpVol,
            .full = false,
            .empty = false,
        };
        int min_vol = sox_excel_input[i].vol[0];
        int max_vol = sox_excel_input[i].vol[0];
        for (int j = 0; j < CELL_NUMS; j++) {
            input.vol[j] = sox_excel_input[i].vol[j];
            if(sox_excel_input[i].vol[j] < min_vol){
                min_vol = sox_excel_input[i].vol[j];
            }
            if(sox_excel_input[i].vol[j] > max_vol){
                max_vol = sox_excel_input[i].vol[j];
            }
        }
        if(max_vol > s_chg_stop_vol)
        {
            input.full = true;
        }
        if(min_vol < s_dsg_stop_vol)
        {
            input.empty = true;
        }
        // asign temp input(map input)
#if CFG_SOX_PORT_SIM_PROJECT == 1
        project_stack_cell_tempature_map(input.tmp, sox_excel_input[i].tmp);
#elif CFG_SOX_PORT_SIM_PROJECT == 2
        project_jiguang_cell_tempature_map(input.tmp, sox_excel_input[i].tmp);
#elif CFG_SOX_PORT_SIM_PROJECT == 3
        cell314_tempature_map(input.tmp, sox_excel_input[i].tmp);
#endif
        // caculate
        sox_task(&input);
#if SOX_DEBUG && SOX_DEBUG_TIME
        printf("time consumption: %u us   <--->  ", get_task_runtime());
        printf("call tick: %u\n", get_task_calltick());
#endif
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
        for (int j = 0; j < CELL_NUMS; j++)
        {
            fprintf(output_cell_soc_simulate_fd, "%f ", cell_soc_output[i].soc[j]);
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



    system("rm output_group_cur_mcu.csv");
    if(compare_pure_AH_SOC){
        // group current(microcontroler sample)
        FILE *output_group_cur_mcu_fd;

        // Open the file for writing(always overwrite)
        output_group_cur_mcu_fd = fopen("output_group_cur_mcu.csv", "w");

        // Write the array to the file
        for (int i = 0; i < s_excel_row-1; i++)
        {
            fprintf(output_group_cur_mcu_fd, "%f ", sox_excel_input[i].cur);
            fprintf(output_group_cur_mcu_fd, "\n");
        }
        fclose(output_group_cur_mcu_fd);
    }






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
    strcat(call_python_cmd, "fish -c \"source /home/hm/Desktop/mysoc/.venv/bin/activate.fish && python3 /home/hm/Desktop/mysoc/test314/soc.py ");      // absolute path, fix next version
    strcat(call_python_cmd,"'");
    strcat(call_python_cmd, whole_cmd);
    strcat(call_python_cmd,"' \"");
    printf("%s\n", call_python_cmd);
    system(call_python_cmd);


    return 0;
}




#if CFG_SOX_PORT_SIM_PROJECT == 1
static void project_stack_cell_tempature_map(int16_t *input, float *output)
{

    input[0] = roundf(output[0]*10);
    for (size_t j = 0; j < 7; j++)
    {
        input[j*2+1] = roundf(output[j+1]*10);
        input[j*2+2] = roundf(output[j+2]*10);
    }
    input[15] = roundf(output[8]*10);
}
#endif

#if CFG_SOX_PORT_SIM_PROJECT == 2
static void project_jiguang_cell_tempature_map(int16_t *input, float *output)
{
    input[0] = roundf(output[0]*10);
    input[1] = roundf(output[1]*10);
    input[2] = roundf(output[2]*10);
    input[3] = roundf(output[2]*10);
    input[4] = roundf(output[3]*10);
    input[5] = roundf(output[4]*10);
}
#endif


#if CFG_SOX_PORT_SIM_PROJECT == 3
static void cell314_tempature_map(int16_t *input, float *output)
{
    input[0] = roundf(output[0]*10);
    input[1] = roundf(output[1]*10);
    input[2] = roundf(output[2]*10);
    input[3] = roundf(output[3]*10);
}
#endif
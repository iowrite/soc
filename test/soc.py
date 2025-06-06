

import os
import struct
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import MaxNLocator
import warnings
import argparse
warnings.filterwarnings("ignore", category=UserWarning, module="openpyxl")


parser = argparse.ArgumentParser(description='Process some SOC data.')
parser.add_argument('filepath', type=str, help='The path to the Excel file')
parser.add_argument('--init_soc', type=float, default=0.0, help='Initial SOC value (default: 0.0)')
parser.add_argument('--enable_stdsoc', action='store_true', help='Enable stdsoc calculation (default: False)')
args = parser.parse_args()



# 读取Excel文件
try:

    # 使用pandas读取Excel文件，默认读取第一个工作表
    df = pd.read_excel(args.filepath)

    # # 将DataFrame转换为二维列表
    # data_list = data_range.values.tolist()
    data_list = df.values.tolist()

except FileNotFoundError:
    print(f"Error: The file  was not found.")
except Exception as e:
    print(f"An error occurred: {e}")

print(len(data_list))

# 提取第 12 列和第 73 到 98 列
combined_list = [[row[9]] + [row[11]] + [row[38]] + row[76:92] + row[108:117]for row in data_list]

# 打印结果以验证
#print(combined_list)

cur = [row[11] for row in data_list]
print(len(cur))
totalAH = sum(cur)


socAH = []

init_soc = args.init_soc
enable_stdsoc = args.enable_stdsoc

if enable_stdsoc:
    socAH.append(init_soc/100*abs(totalAH))
    cumulative_sum = socAH[0]
    for current in cur:
        cumulative_sum += current
        socAH.append(cumulative_sum)

# print("socAH:", socAH)

# try:
#     # 使用pandas读取Excel文件中的特定列
#     df = pd.read_excel('data/not_fix_temp/std_chg_0.5c.xlsx', usecols=['cap'])
#     # 将DataFrame转换为一维列表
#     stdData = df['cap'].tolist()
# except FileNotFoundError:
#     print(f"Error: The file '{'std_chg_0.5c.xlsx'}' was not found.")
# except Exception as e:
#     print(f"An error occurred: {e}")

# print(len(stdData))


#############################################################################################333




# Create the FIFO if it doesn't exist
if not os.path.exists("socfifo_write_input_row_len"):
    os.mkfifo("socfifo_write_input_row_len")

# Open the FIFO for writing
with open("socfifo_write_input_row_len", 'wb') as fifo:
    # Pack the array of integers into binary data
    binary_data = struct.pack('i', len(data_list))
    # Write the binary data to the FIFO
    fifo.write(binary_data)

print("send input data row len")


#############################################################################################333

# Create the FIFO if it doesn't exist
if not os.path.exists("socfifo_write_input"):
    os.mkfifo("socfifo_write_input")

flattened_list = [item for sublist in combined_list for item in sublist]

# print(flattened_list)
print(len(flattened_list))
print(len(data_list))

# Open the FIFO for writing
with open("socfifo_write_input", 'wb') as fifo:
    # Pack the array of integers into binary data
    binary_data = struct.pack('ffffffffffffffffffffffffffff' * len(data_list), *flattened_list)
    # Write the binary data to the FIFO
    fifo.write(binary_data)

print("Array sent successfully")





#############################################################################################333



# Wait until the FIFO exists
while not os.path.exists('socfifo_output'):
    pass

# Open the FIFO for reading
with open('socfifo_output', 'rb') as fifo:
    # Read the binary data from the FIFO
    binary_data = fifo.read((len(data_list)+1) * 32)  
    print(len(binary_data))
    # Unpack the binary data into an array of integers
    received_array = list(struct.unpack('H' *16* (len(data_list)+1), binary_data))

print("cel soc recv output")





# Wait until the FIFO exists
while not os.path.exists('grpsocfifo_output'):
    pass

# Open the FIFO for reading
with open('grpsocfifo_output', 'rb') as fifo:
    # Read the binary data from the FIFO
    binary_data = fifo.read((len(data_list)+1) * 8)  

    # Unpack the binary data into an array of integers
    received_array_grp = list(struct.unpack('HHHH' * (len(data_list)+1), binary_data))

print("grp soc recv output")







def list_to_2d(input_list, cols):
    return [input_list[i:i + cols] for i in range(0, len(input_list), cols)]


two_dimensional_list = list_to_2d(received_array, 16)
# print(two_dimensional_list)

two_dimensional_list_grp = list_to_2d(received_array_grp, 4)
print(two_dimensional_list_grp)


soc_output = [row[0] for row in two_dimensional_list]
# print(soc_output)
# soc_er2_output = [row[1] for row in received_array]

output_x = list(range(0, len(two_dimensional_list)))

# 创建图形和轴对象
# stdData_multiplied_rounded = [round(x * 10) for x in stdData]
y_ticks = np.arange(-5, 105, 5)
# 绘制散点图
for i in range(0, 16):
    plt.subplot(4, 4, i+1)
    plt.plot(output_x, [row[i] for row in two_dimensional_list], label=f"soc cel {i+1}")
    # plt.plot(stdData_multiplied_rounded, label=f"std", linestyle='--', color='gray')
    
    # 显示网格
    plt.grid(True)
    plt.yticks(y_ticks)


    # 显示图例
    plt.legend()

plt.ylim(-5, 105)


plt.figure()

plt.grid(True)
plt.yticks(y_ticks)
plt.plot(output_x, [row[0] for row in two_dimensional_list_grp], label="grp soc")
plt.plot(output_x, [row[1] for row in two_dimensional_list_grp], label="max soc")
plt.plot(output_x, [row[2] for row in two_dimensional_list_grp], label="min soc")
plt.plot(output_x, [row[3] for row in two_dimensional_list_grp], label="avg soc")
if  enable_stdsoc:
    plt.plot(output_x, [AH/abs(totalAH)*100 for AH in socAH], label="pureAH soc", color='grey')
plt.ylim(-5, 105)
plt.legend()


if enable_stdsoc:
    # Calculate the difference between grp soc and pureAH soc
    difference = [abs(row[0] - (AH/abs(totalAH)*100)) for row, AH in zip(two_dimensional_list_grp, socAH)]

    # Create a new figure for the difference
    fig, ax1 = plt.subplots()

    # Plot grp soc and pureAH soc as reference
    ax1.plot(output_x, [row[0] for row in two_dimensional_list_grp], label="grp soc", color='blue')
    ax1.plot(output_x, [AH/abs(totalAH)*100 for AH in socAH], label="pureAH soc", color='grey')
    ax1.set_ylim(-5, 105)
    ax1.set_ylabel("SOC (%)")
    ax1.yaxis.label.set_color('blue')

    # Create a second y-axis for the difference
    ax2 = ax1.twinx()
    ax2.plot(output_x, difference, label="Difference", color='red')
    ax2.set_ylim(0, 10)  # Set the y-axis range to 0% to 10%
    ax2.set_yticks(np.arange(0, 11, 1))
    ax2.set_ylabel("Difference (%)")
    ax2.yaxis.label.set_color('red')

    # Show grid and legend
    ax2.grid(True, axis='y', linestyle='--', alpha=0.6)
    ax1.figure.legend(loc='upper right', bbox_to_anchor=(1,1), bbox_transform=ax1.transAxes)

    plt.title("grp soc vs pureAH soc with Difference")

    print(totalAH/3600)

plt.show()



# # 保存图形到文件
# plt.savefig('/home/matlab/Documents/soc/scatter_plot.png')

# # 关闭图形以释放内存
# plt.close()











import os
import struct
import pandas as pd
import matplotlib.pyplot as plt

# 指定Excel文件的路径
file_path = 'cellchgl.xlsx'

# 读取Excel文件
try:

    # 使用pandas读取Excel文件，默认读取第一个工作表
    df = pd.read_excel(file_path)

    # # 将DataFrame转换为二维列表
    # data_list = data_range.values.tolist()
    data_list = df.values.tolist()

except FileNotFoundError:
    print(f"Error: The file '{file_path}' was not found.")
except Exception as e:
    print(f"An error occurred: {e}")



print(len(data_list))



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

flattened_list = [item for sublist in data_list for item in sublist]

# print(flattened_list)
print(len(flattened_list))
print(len(data_list))

# Open the FIFO for writing
with open("socfifo_write_input", 'wb') as fifo:
    # Pack the array of integers into binary data
    binary_data = struct.pack('fffffffffffffffff' * len(data_list), *flattened_list)
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

    # Unpack the binary data into an array of integers
    received_array = list(struct.unpack('H' *16* (len(data_list)+1), binary_data))

print("recv output")



def list_to_2d(input_list, cols):
    return [input_list[i:i + cols] for i in range(0, len(input_list), cols)]


two_dimensional_list = list_to_2d(received_array, 16)
print(two_dimensional_list)


soc_output = [row[0] for row in two_dimensional_list]
# print(soc_output)
# soc_er2_output = [row[1] for row in received_array]

output_x = list(range(0, len(two_dimensional_list)))

# 创建图形和轴对象
plt.figure(figsize=(8, 6))

# 绘制散点图
plt.plot(output_x, soc_output, label="soc cel 1")

# 添加标题和标签
plt.title('Scatter Plot of Two-Column Data')
plt.xlabel('X Coordinate')
plt.ylabel('Y Coordinate')
plt.ylim(-50, 1050)

# 显示网格
plt.grid(True)

# 显示图例
plt.legend()

plt.show()

# # 保存图形到文件
# plt.savefig('/home/matlab/Documents/soc/scatter_plot.png')

# # 关闭图形以释放内存
# plt.close()










import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 读取CSV文件
df_cell_soc = pd.read_csv('output_cell_soc_simulate.csv', sep=' ', header=None, skip_blank_lines=True).astype(float)   # 没有标题行
df_grp_soc_sim  = pd.read_csv('output_group_soc_simulate.csv', sep=' ', header=None, skip_blank_lines=True).astype(float)
df_grp_soc_mcu  = pd.read_csv('output_group_soc_mcu.csv', sep=' ', header=None, skip_blank_lines=True).astype(float)
if os.path.exists("output_group_cur_mcu.csv"):
    df_grp_cur_mcu  = pd.read_csv('output_group_cur_mcu.csv', sep=' ', header=None, skip_blank_lines=True).astype(float)
    # 积分累加
    pure_AH = np.cumsum(df_grp_cur_mcu[0])
    # print(pure_AH)
    print("charge or discharge {} AH", pure_AH.iloc[-1]/3600)
    pure_AH_soc = pure_AH/pure_AH.iloc[-1]*100
    if pure_AH.iloc[-1] < 0:
        pure_AH_soc = 100 - pure_AH_soc
        


    






# 计算每行的统计量
row_min = df_cell_soc.min(axis=1)
row_max = df_cell_soc.max(axis=1)
row_mean = df_cell_soc.mean(axis=1)

# 创建图表
plt.figure(figsize=(16, 9))
# 设置窗口标题为脚本的第一个参数（如果提供了的话）
if len(sys.argv) > 1:
    window_title = " ".join(sys.argv[1:])  # 合并所有参数
else:
    window_title = "Cell SOC Simulation"  # 默认名称
plt.gcf().canvas.manager.set_window_title(window_title)



plt.subplot(1,2,1)
# 绘制每一列的数据
for i in range(df_cell_soc.shape[1]):
    plt.plot(df_cell_soc[i], label=f'cell {i+1}')
plt.xlabel('excel input step(row)')
plt.ylabel('soc')
plt.title('cell soc')
plt.legend()
# 添加网格和y轴刻度
plt.grid(axis='y', linestyle='--')
plt.yticks(range(0, 101, 5))

plt.subplot(1,2,2)
# 添加统计量的曲线
plt.plot(row_min, label='Min SOC', linewidth=2, color='blue')
plt.plot(row_max, label='Max SOC', linewidth=2, color='green')
plt.plot(row_mean, label='Average SOC', linewidth=2,color='red')
plt.plot(df_grp_soc_sim[0], label='simulate SOC', linewidth=2,color='black')
# plt.plot(df_grp_soc_mcu, label='mcu SOC', linewidth=2,color='gray')
if os.path.exists("output_group_cur_mcu.csv"):
    plt.plot(pure_AH_soc, label='pure AH SOC', linewidth=2,color='gray')


plt.xlabel('excel input step(row)')
plt.ylabel('soc')
plt.title('group soc')
plt.legend()
# 添加网格和y轴刻度
plt.grid(axis='y', linestyle='--')
plt.yticks(range(0, 101, 5))

# 保存图片
output_image_path = window_title+'.png' 
output_image_path = output_image_path.replace(" ", "_")
output_image_path = output_image_path.replace("/", "@")
plt.savefig(output_image_path, dpi=300, bbox_inches='tight')
print(f"图片已保存至: {output_image_path}")

plt.show()


















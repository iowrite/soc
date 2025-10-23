import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.widgets import CheckButtons
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
for i in range(df_cell_soc.shape[1]-1):
    plt.plot(df_cell_soc[i], label=f'cell {i+1}')
plt.xlabel('excel input step(row)')
plt.ylabel('soc')
plt.title('cell soc')
plt.legend()
# 添加网格和y轴刻度
plt.grid(axis='y', linestyle='--')
plt.yticks(range(0, 101, 5))

ax = plt.subplot(1,2,2)
# 创建第二个y轴（右侧）
ax2 = ax.twinx()
# 添加统计量的曲线
line_min, = ax.plot(row_min, label='Min SOC', linewidth=2, color='blue')
line_max, = ax.plot(row_max, label='Max SOC', linewidth=2, color='green')
line_avg, = ax.plot(row_mean, label='Average SOC', linewidth=2,color='red')
line_sim, = ax.plot(df_grp_soc_sim[0], label='simulate SOC', linewidth=2,color='black')
# plt.plot(df_grp_soc_mcu, label='mcu SOC', linewidth=2,color='gray')
if os.path.exists("output_group_cur_mcu.csv"):
    line_ah, = ax.plot(pure_AH_soc, label='pure AH SOC', linewidth=2,color='gray')
    diff_data = np.abs(df_grp_soc_sim[0] - pure_AH_soc)
    line_ah_diff, = ax2.plot(diff_data, label='sim diff', linewidth=2,color='orange')
    # 设置第二个y轴的范围为0到10
    ax2.set_yticks(range(0, 10, 1))
    ax2.set_ylabel('Difference')

ax.set_xlabel('excel input step(row)')
ax.set_ylabel('soc')
ax.set_title('group soc')
ax.legend()
# 添加网格和y轴刻度
ax.grid(axis='y', linestyle='--')
ax.set_yticks(range(0, 101, 5))



# 创建复选框区域
rax = plt.axes([0.9, 0.8, 0.08, 0.08])  # [left, bottom, width, height]
check = CheckButtons(
    ax=rax,
    labels=['line_min', 'line_max', 'line_avg', 'line_sim'],
    actives=[True, True, True, True]
)

# 定义复选框回调函数
def func(label):
    if label == 'line_min':
        line_min.set_visible(not line_min.get_visible())
    elif label == 'line_max':
        line_max.set_visible(not line_max.get_visible())
    elif label == 'line_avg':
        line_avg.set_visible(not line_avg.get_visible())
    elif label == 'line_sim':
        line_sim.set_visible(not line_sim.get_visible())
    plt.draw()

# 连接回调函数
check.on_clicked(func)




# 保存图片
output_image_path = window_title+'.png' 
output_image_path = output_image_path.replace(" ", "_")
output_image_path = output_image_path.replace("/", "@")
plt.savefig(output_image_path, dpi=300, bbox_inches='tight')
print(f"图片已保存至: {output_image_path}")

plt.show()


















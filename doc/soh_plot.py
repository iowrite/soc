import numpy as np
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection

cycles = list(range(1, 5000))

soh_low_temp = np.linspace(100, 80, num=len(cycles))
soh_high_temp = np.linspace(100, 60, num=len(cycles))
soh_real = np.linspace(100, 70, num=len(cycles))
soh_real_tune = np.sin(np.array(cycles)/100)
soh_real_decrease_rate = soh_real_tune+1
soh_diff = np.diff(soh_real)
soh_diff = np.insert(soh_diff, 0, 0)
soh_diff2 = soh_real_decrease_rate * soh_diff
soh_diff2[0] = 100
soh_normal = np.cumsum(soh_diff2)

plt.figure(figsize=(10, 6))

# 绘制参考线
plt.plot(cycles, soh_low_temp, label='SOH low temp', color='blue')
plt.plot(cycles, soh_high_temp, label='SOH high temp', color='orange')

# 创建渐变色线段
points = np.array([cycles, soh_normal]).T.reshape(-1, 1, 2)
segments = np.concatenate([points[:-1], points[1:]], axis=1)

# 计算下降速度（斜率）
slopes = np.diff(soh_normal)

# 使用颜色映射：下降越快颜色越接近orange，下降越慢越接近blue
norm = plt.Normalize(slopes.min(), slopes.max())
# 创建从blue到orange的颜色映射，并反转方向
lc = LineCollection(segments, cmap='coolwarm_r', norm=norm, linewidth=2)  # 添加_r反转颜色映射
lc.set_array(slopes)
plt.gca().add_collection(lc)

# 添加颜色条
cbar = plt.colorbar(lc, label='Degradation Speed')
# 明确标注颜色条的方向
cbar.ax.invert_yaxis()  # 确保颜色条也正确显示：顶部为快速下降（橙色），底部为慢速下降（蓝色）

plt.xlabel('Cycles')
plt.ylabel('State of Health (%)')
plt.title('Battery SOH Degradation with Speed-based Coloring')
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()
plt.show()
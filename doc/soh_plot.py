import numpy as np
import matplotlib.pyplot as plt

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
plt.plot(cycles, soh_normal)

plt.xlabel('Cycles')
plt.ylabel('State of Health (%)')
plt.legend()
plt.grid()
plt.show()
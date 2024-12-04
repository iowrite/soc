import pandas as pd
import warnings
import matplotlib.pyplot as plt
import numpy as np
from numpy.polynomial.polynomial import Polynomial
warnings.filterwarnings("ignore", category=UserWarning, module="openpyxl")


def process_curve(file_path, data_points):
    columns_to_read = ['时间', '组端电流', "平均电压"]  # 替换为你要读取的具体列名

    df = pd.read_excel(file_path, usecols=columns_to_read)

    cap = 0
    cap_voltage_dict = {}  
    for index, row in df.iterrows():
        cap = cap + row['组端电流']/3600
        cap_voltage_dict[cap] = row['平均电压']

    # print(cap_voltage_dict)

    # 获取 cap_voltage_dict 的最后一个元素
    # first_key = list(cap_voltage_dict.keys())[0]
    # first_value = cap_voltage_dict[first_key]
    # print(f"第一个元素: 容量: {first_key}, 平均电压: {first_value}")
    last_key = list(cap_voltage_dict.keys())[-1]
    # last_value = cap_voltage_dict[last_key]
    # print(f"最后一个元素: 容量: {last_key}, 平均电压: {last_value}")

    # percent1 = last_key/100
    # print(f"1% per soc cap is {percent1}")

    # 生成从 0 开始，间隔为 percent1，长度为 101 的数组
    array_linspace = np.linspace(0, last_key, data_points)

    # 初始化一个字典来存储每个整数值最接近的 cap 值及其对应的平均电压
    closest_cap_voltage_dict = {i: (float('inf'), float('inf')) for i in range(data_points)}

    # print(closest_cap_voltage_dict)

    # 遍历 cap_voltage_dict
    for idx, i in enumerate(array_linspace):
        # print(i)
        min_distance = float('inf')
        for cap, voltage in cap_voltage_dict.items():
            distance = abs(cap - i)
            if distance < min_distance:
                min_distance = distance
                closest_cap_voltage_dict[idx] = (cap, voltage)

    # 打印结果
    # for i in range(101):
    #     closest_cap, closest_voltage = closest_cap_voltage_dict[i]
    #     print(f"整数值: {i}, 最接近的 cap: {closest_cap}, 对应的平均电压: {closest_voltage}")


    # 提取平均电压到列表中
    voltage_list = [closest_cap_voltage_dict[i][1] for i in range(data_points)]


    return voltage_list


def fit_quadratic_and_get_slopes(voltage_list):
    slopes = []
    n = len(voltage_list)
    
    for i in range(n - 2):
        x = np.array([i, i+1, i+2])
        y = np.array([voltage_list[i], voltage_list[i+1], voltage_list[i+2]])
        
        # 拟合二次函数
        quad_fit = Polynomial.fit(x, y, 1)
        
        # 计算导数
        quad_fit_derivative = quad_fit.deriv()
        
        # 计算三个点处的斜率
        slope_at_i = quad_fit_derivative(i)
        slope_at_i1 = quad_fit_derivative(i+1)
        slope_at_i2 = quad_fit_derivative(i+2)
        
        slopes.extend([slope_at_i, slope_at_i1, slope_at_i2])
    
    # print("斜率列表: (每个点处的斜率) ", slopes)
    # 去除重复的斜率（每个点会被计算两次）
    unique_slopes = slopes[::3]
    unique_slopes = [max(slope, 0.5) for slope in unique_slopes]
    # print(len(unique_slopes))
    unique_slopes.append(unique_slopes[-1])
    unique_slopes.append(unique_slopes[-1])

    voltage_list_py = [float(item) for item in unique_slopes]
    return voltage_list_py







# 示例调用
if __name__ == "__main__":
    filepath = 'data/25d5.xlsx'
    plt.subplot(2,2,1)
    voltage_list = process_curve(filepath, 101)
    plt.plot(voltage_list)
    print("平均电压列表: (101 points)\n", voltage_list)

    print("\n============================\n")

    plt.subplot(2,2,2)
    slopes = fit_quadratic_and_get_slopes(voltage_list)
    formatted_slopes = [f"{s:.1f}" for s in slopes]
    print("斜率列表: (101 points)\n", slopes)
    plt.plot(slopes)


    print("\n============================\n")

    plt.subplot(2,2,3)
    voltage_list_21 = slopes[::5]
    plt.plot([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100],voltage_list_21)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100],voltage_list_21)
    formatted_voltage_list_21 = [f"{v:.1f}" for v in voltage_list_21]
    print("斜率列表: (21 points)\n", voltage_list_21)

    print("\n============================\n")

    plt.subplot(2,2,4)
    voltage_list = process_curve(filepath, 21)
    plt.plot([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100], voltage_list)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100],voltage_list)
    print("平均电压列表: (21 points)\n", voltage_list)

    print("\n============================\n")


    plt.show()




    




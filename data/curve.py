import pandas as pd
import warnings
import matplotlib.pyplot as plt
import numpy as np
from numpy.polynomial.polynomial import Polynomial
import argparse
warnings.filterwarnings("ignore", category=UserWarning, module="openpyxl")
RED = "\033[91m"
RESET = "\033[0m"

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
    last_value = cap_voltage_dict[last_key]
    print(f"最后一个元素: 容量: {last_key}, 平均电压: {last_value}")

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
    

    sfirst = voltage_list[1]-voltage_list[0]
    slopes.append(sfirst)

    for i in range(1, n - 1):
        x = np.array([i-1, i, i+1])
        y = np.array([voltage_list[i-1], voltage_list[i], voltage_list[i+1]])
        
        # 拟合一次函数
        quad_fit = Polynomial.fit(x, y, 1)

        # print(f"拟合后的多项式函数 f(x) = {quad_fit}")
        fit_value_at_i = quad_fit(i-1)
        fit_value_at_i1 = quad_fit(i)
        fit_value_at_i2 = quad_fit(i+1)
        # print(f"f({i-1}) = {fit_value_at_i}, f({i}) = {fit_value_at_i1}, f({i+1}) = {fit_value_at_i2}")
        
        # 计算导数
        quad_fit_derivative = quad_fit.deriv()
        
        # 计算三个点处的斜率
        slope_at_i = quad_fit_derivative(i)

        
        slopes.append(slope_at_i)
    
    # print("斜率列表: (每个点处的斜率) ", slopes)
    # 去除重复的斜率（每个点会被计算两次）

    

    slast = voltage_list[-1]-voltage_list[-2]
    slopes.append(slast)
    for i, slope in enumerate(slopes):
        if(abs(slope) < 0.2):
            if(slopes[0] > 0):
                slopes[i] = 0.2
            else:
                slopes[i] = -0.2

    voltage_list_py = [float(item) for item in slopes]
    return voltage_list_py







# 示例调用
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Process some curve data.')
    parser.add_argument('filepath', type=str, help='The path to the Excel file')
    args = parser.parse_args()

    filepath = args.filepath
    plt.subplot(2,2,1)
    voltage_list = process_curve(filepath, 101)
    voltage_list_cp = voltage_list.copy()
    plt.plot(voltage_list)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100], voltage_list[::5])
    if(voltage_list[0] > voltage_list[1]):
        voltage_list.reverse()
        print("平均电压列表(从小到大): (101 points)\n", voltage_list)
        plt.grid(axis='y', linestyle='--')
        plt.yticks(range(2800, 3500, 100))
    else:
        print("平均电压列表: (101 points)\n", voltage_list)
        plt.grid(axis='y', linestyle='--')
        plt.yticks(range(3200, 3600, 50))
    # 添加网格和y轴刻度

    print("\n============================\n")

    plt.subplot(2,2,2)
    slopes = fit_quadratic_and_get_slopes(voltage_list)
    slopes_round = [round(slope * 10) for slope in slopes]
    print("斜率列表: (101 points)\n", slopes_round)
    plt.plot(slopes)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100], slopes[::5])


    print("\n============================\n")

    plt.subplot(2,2,3)
    voltage_list_21 = slopes[::5]
    plt.plot([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100],voltage_list_21)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100],voltage_list_21)
    voltage_list_21 = [round(v * 10) for v in voltage_list_21]
    print("斜率列表(乘10): (21 points)\n", voltage_list_21)
    voltage_list_29 = voltage_list_21.copy()
    voltage_list_29.insert(1, slopes_round[1])
    voltage_list_29.insert(2, slopes_round[2])
    voltage_list_29.insert(3, slopes_round[3])
    voltage_list_29.insert(4, slopes_round[4])
    voltage_list_29.insert(24, slopes_round[96])
    voltage_list_29.insert(25, slopes_round[97])
    voltage_list_29.insert(26, slopes_round[98])
    voltage_list_29.insert(27, slopes_round[99])
    print("斜率列表(乘10): (29 points)\n", voltage_list_29)


    # 检查 voltage_list 中是否有负值
    has_negative = any(v < 0 for v in voltage_list_29)
    if has_negative:
        print(f"{RED}斜率有负值{RESET}")
    print("\n============================\n")

    plt.subplot(2,2,4)
    voltage_list21 = voltage_list_cp[::5]
    plt.plot([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100], voltage_list21)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100],voltage_list21)
    print("平均电压列表: (21 points)\n", voltage_list21)
    voltage_list29 = voltage_list21.copy()
    if(voltage_list29[0] > voltage_list[1]):
        voltage_list29.reverse()
    voltage_list29.insert(1, voltage_list[1])
    voltage_list29.insert(2, voltage_list[2])
    voltage_list29.insert(3, voltage_list[3])
    voltage_list29.insert(4, voltage_list[4])
    voltage_list29.insert(24, voltage_list[96])
    voltage_list29.insert(25, voltage_list[97])
    voltage_list29.insert(26, voltage_list[98])
    voltage_list29.insert(27, voltage_list[99])
    print("平均电压列表: (29 points)\n", voltage_list29)
    unique_values = set(voltage_list29)
    if len(unique_values) != len(voltage_list29):
        print(f"{RED}平均电压列表中有相同的值{RESET}")
    print("\n============================\n")


    plt.show()




    




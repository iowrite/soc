import pandas as pd
import warnings
import matplotlib.pyplot as plt
import numpy as np
from numpy.polynomial.polynomial import Polynomial
import argparse
import sys
import os
# Get the directory of the current script
current_dir = os.path.dirname(os.path.abspath(__file__))
# Go up one level to /home/hm/Desktop/mysoc/data
parent_dir = os.path.dirname(current_dir)
sys.path.insert(0, parent_dir)
from curve import fit_quadratic_and_get_slopes


def simple_normalize(data, target_points=101):
    # 移除NaN值
    clean_data = data.dropna().values if hasattr(data, 'dropna') else data[~np.isnan(data)]
    
    # 计算原数据的索引
    original_indices = np.arange(len(data))
    clean_indices = original_indices[~np.isnan(data)] if hasattr(data, 'values') else np.where(~np.isnan(data))[0]
    
    # 选择等间隔的点
    indices = np.linspace(0, len(clean_data)-1, target_points, dtype=int)
    
    normalized_data = clean_data[indices]
    corresponding_indices = clean_indices[indices]
    
    return normalized_data, corresponding_indices

def read_excel_analysis(file_path):
    # 获取所有sheet名称
    excel_file = pd.ExcelFile(file_path)
    sheet_names = excel_file.sheet_names
    
    print(f"文件包含 {len(sheet_names)} 个sheet: {sheet_names}")
    
    # 读取所有sheet
    all_data = {}
    for sheet_name in sheet_names:
        df = pd.read_excel(file_path, sheet_name=sheet_name)
        all_data[sheet_name] = df
        
    
    return all_data

def main():
    # 创建参数解析器
    parser = argparse.ArgumentParser(description='分析电池电压数据')
    parser.add_argument('file_path', type=str, help='Excel文件路径')
    
    # 解析参数
    args = parser.parse_args()
    
    # 使用参数中的文件路径
    file_path = args.file_path
    print("===================================================================")
    print("processing file:", file_path)
    print("===================================================================")
    data_dict = read_excel_analysis(file_path)

    sheet1 = list(data_dict.keys())[0]
    df1 = data_dict[sheet1]
    vol1 = df1.iloc[:, 7-1]
    cap1_array = df1.iloc[:, 9-1]
    cap1 = cap1_array.iloc[-1]
    print("sheet1 row lenth is: ", len(vol1))

    sheet2 = list(data_dict.keys())[1]
    df2 = data_dict[sheet2]
    vol2 = df2.iloc[:, 7-1]
    cap2_array = df2.iloc[:, 9-1]
    cap2 = cap2_array.iloc[-1]

    sheet3 = list(data_dict.keys())[2]
    df3 = data_dict[sheet3]
    vol3 = df3.iloc[:, 7-1]
    cap3_array = df3.iloc[:, 9-1]
    cap3 = cap3_array.iloc[-1]

    sheet4 = list(data_dict.keys())[3]
    df4 = data_dict[sheet4]
    vol4 = df4.iloc[:, 7-1]
    cap4_array = df4.iloc[:, 9-1]
    cap4 = cap4_array.iloc[-1]


    print("capacity is: ", cap1, cap2, cap3, cap4)
    print("avg_capacity is: ", (cap1+cap2+cap3+cap4)/4)
        

    plt.figure()
    plt.get_current_fig_manager().set_window_title(file_path)

    fig1 = plt.subplot(2,2,1)

    fig1.plot(vol1*1000, label="cell1 vol", alpha=0.3)
    fig1.plot(vol2*1000, label="cell2 vol", alpha=0.3)
    fig1.plot(vol3*1000, label="cell3 vol", alpha=0.3)
    fig1.plot(vol4*1000, label="cell4 vol", alpha=0.3)

    fig1.legend()



    vol1_len = len(vol1)
    vol2_len = len(vol2)
    vol3_len = len(vol3)
    vol4_len = len(vol4)

    lengths = [vol1_len, vol2_len, vol3_len, vol4_len]
    min_length = min(lengths)
    # print(lengths)



    # 将所有电压曲线截断为最小长度
    vol1_truncated = vol1.iloc[:min_length] if hasattr(vol1, 'iloc') else vol1[:min_length]
    vol2_truncated = vol2.iloc[:min_length] if hasattr(vol2, 'iloc') else vol2[:min_length]
    vol3_truncated = vol3.iloc[:min_length] if hasattr(vol3, 'iloc') else vol3[:min_length]
    vol4_truncated = vol4.iloc[:min_length] if hasattr(vol4, 'iloc') else vol4[:min_length]

    # print(vol1_truncated)
    # print(vol2_truncated)
    # print(vol3_truncated)
    # print(vol4_truncated)


    # 计算截断后曲线的平均值
    # 将四个截断后的曲线转换为numpy数组并堆叠
    truncated_arrays = np.array([
        vol1_truncated.values if hasattr(vol1_truncated, 'values') else vol1_truncated,
        vol2_truncated.values if hasattr(vol2_truncated, 'values') else vol2_truncated,
        vol3_truncated.values if hasattr(vol3_truncated, 'values') else vol3_truncated,
        vol4_truncated.values if hasattr(vol4_truncated, 'values') else vol4_truncated
    ])

    # 计算平均值（沿着第0轴，即对四个cell求平均）
    average_curve = np.mean(truncated_arrays, axis=0)

    # print(average_curve)



    fig2 = plt.subplot(2,2,2)
    vol1_101,_ = simple_normalize(vol1)
    vol2_101,_ = simple_normalize(vol2)
    vol3_101,_ = simple_normalize(vol3)
    vol4_101,_ = simple_normalize(vol4)

    volavg_101, volavg_101_idx = simple_normalize(average_curve)
    # print(volavg_101)
    # print(volavg_101_idx)
    volavg_101_99percent,_ = simple_normalize(average_curve[:volavg_101_idx[-2]+1])
    volavg_101_99percent = volavg_101_99percent*1000
    volavg_101_99percent = np.round(volavg_101_99percent)
    print("电压列表, 101 point ", volavg_101_99percent)
    voltage_list_21 = volavg_101_99percent[::5]
    voltage_list_29 = voltage_list_21.copy()
    voltage_list_29 = np.insert(voltage_list_29, 1, volavg_101_99percent[1:5])
    voltage_list_29 = np.insert(voltage_list_29, 24, volavg_101_99percent[96:100])
    print("电压列表, 21 point ", voltage_list_21)
    print("电压列表, 29 point ", voltage_list_29)


    fig2.plot(vol1_101*1000, label="cell1 vol", alpha=0.3)
    fig2.plot(vol2_101*1000, label="cell2 vol", alpha=0.3)
    fig2.plot(vol3_101*1000, label="cell3 vol", alpha=0.3)
    fig2.plot(vol4_101*1000, label="cell4 vol", alpha=0.3)

    fig2.legend()





    fig3 = plt.subplot(2,2,3)
    fig3.plot(volavg_101*1000, label="average vol", alpha=0.3)
    fig3.plot(volavg_101_99percent, label="average vol 99%", alpha=0.3)
    plt.scatter([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100], volavg_101_99percent[::5])

    fig3.legend()



    fig4 = plt.subplot(2,2,4)
    volavg_101_slope = fit_quadratic_and_get_slopes(np.round(volavg_101_99percent))
    fig4.plot(volavg_101_slope)
    volavg_101_slope = np.round([point *10 for point in volavg_101_slope])
    print("斜率列表: (101 points)\n", volavg_101_slope)

    slope_list_21 = volavg_101_slope[::5]
    slope_list_29 = slope_list_21.copy()
    slope_list_29 = np.insert(slope_list_29, 1, volavg_101_slope[1:5])
    slope_list_29 = np.insert(slope_list_29, 24, volavg_101_slope[96:100])
    print("斜率列表, 21 point ", slope_list_21)
    print("斜率列表, 29 point ", slope_list_29)











    plt.show()

if __name__ == "__main__":
    main()
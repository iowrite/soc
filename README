# SOC 模块功能设计
## 功能介绍
本模块使用 AEKF 方法, 估算单体 SOC 信息. 由单体 SOC 信息估算组端 SOC. 使用循环次数估算单体 SOH, 由单体 SOH 计算组端 SOH. SOP 由电芯手册得出. SOE 由直接计算所得.
## 代码结构
![](doc/code_structure.png)

## 功能实现
### SOC
对单电芯使用安时积分作为状态方程, 使用终端电压作为观察量. 由于使用模型(一次,二次RC模型等)来拟合终端电压与 SOC 的对应关系比较复杂, 可靠性比较低, 验证周期也比较长, 参数多, 需要的计算, 存储资源大, 本程序直接使用各个电流温度工况下的测量值来近似对应 SOC 下的终端电压. 这样, 只需测量,系列温度,系列电流情况下的采集电压即可, 大大减小了算法的复杂性.
![](doc/curve_matrix.png)

**状态方程:**

![](doc/state_equation.png)
w 为过程噪声

**观测方程:**

![](doc/observation_equation.png)
v 为观测噪声

**预测:**

先验估计 SOC

![](doc/calculate_soc.png)

先验估计 SOC 方差

![](doc/calculate_p.png)
Q 为过程噪声方差

**更新:**

对观测方程线性化, 求观测量 vol 对 soc 的偏导, 这部分可以预先对 电流-温度 充放电曲线表(soc-vol) 求导得到

![](doc/partial_derivative.png)

卡尔曼增益

![](doc/K.png)
R 为观测噪声方差

最优估计 SOC

![](doc/optimal_estimate_soc.png)

后验估计 SOC 方差

![](doc/update_p.png)


### SOH

### SOE

### SOP

### PORT LAYER
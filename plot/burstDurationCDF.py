import matplotlib.pyplot as plt
import numpy as np

# 读取文件数据
with open('../Output/burstDuration.out', 'r') as f:
    lines = f.readlines()

# 解析x和y的值
data = [list(map(int, line.strip().split())) for line in lines]
data.sort(key=lambda x: x[0])  # 根据x值排序

x_values = [x[0] for x in data]
y_values = [x[1] for x in data]

# 计算CDF
y_cdf = np.cumsum(y_values)
y_cdf = y_cdf / y_cdf[-1]

# 绘制图形
plt.figure(figsize=(10, 6))
plt.semilogx(x_values, y_cdf, marker='o')  # 对x取对数
plt.xlabel('burst Duration(1ms) (threashold=1ms)')
plt.ylabel('CDF')
# plt.title('CDF plot with log-scaled X axis')
plt.grid(True, which="both", ls="--")

plt.show()

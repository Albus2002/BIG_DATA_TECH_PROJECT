import numpy as np
import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

# 从文件读取数据
def read_data_from_file(file_path):
    with open(file_path, 'r') as file:
        data = file.readlines()
    return [int(line.strip()) for line in data]

# 计算频率分布
def calculate_distribution(data, bins):
    hist, bin_edges = np.histogram(data, bins=bins, range=(0, 10000), density=False)
    proportion = hist / sum(hist)  # 计算频率占比
    return bin_edges[:-1], proportion

# 绘制柱状图
def plot_distribution(bins, proportions):
    plt.bar(bins, proportions, width=500, align='edge', edgecolor='black', color = cst.style15.color3)
    plt.xlabel('Active Streams(100ms)')
    plt.ylabel('Proportion')
    plt.title('Distribution of Active Streams')
    # plt.xticks(ticks=bins, labels=[f"{int(x)}-{int(x+2000)}" for x in bins])
    plt.xticks(ticks=np.arange(0, 10001, 5000))
    plt.show()

# 主函数
def main():
    file_path = '../activeflownum.txt'  # 更改为你的文件路径
    data = read_data_from_file(file_path)
    bins = np.arange(0, 10000, 500)  # 创建间隔为2000的bins
    bin_edges, proportions = calculate_distribution(data, bins)
    plot_distribution(bin_edges, proportions)

if __name__ == "__main__":
    main()

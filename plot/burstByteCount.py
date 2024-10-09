import matplotlib.pyplot as plt


# 从文件中读取数据
def read_data(filename):
    data = []
    with open(filename, 'r') as file:
        for line in file:
            x, y = map(int, line.split())
            data.append((x, y))
    return data


# 将x按100分隔并统计每个区间内的y值的数量
def compute_histogram(data):
    histogram = {}

    for x, y in data:
        bin_key = (x // 100)
        histogram[bin_key] = histogram.get(bin_key, 0) + 1

    # 将字典转化为两个列表
    x_bins = sorted(histogram.keys())
    y_counts = [histogram[bin_key] for bin_key in x_bins]

    return x_bins, y_counts


# 绘制图形
def plot_data(x_bins, y_counts):
    plt.plot(x_bins, y_counts, '-o', label="Count of y")
    plt.xlabel("x (binned by 100)")
    plt.ylabel("Count of y")
    plt.title("Histogram of y values binned by x")
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    data = read_data("../Output/burstByteCount.out")
    x_bins, y_counts = compute_histogram(data)
    plot_data(x_bins, y_counts)

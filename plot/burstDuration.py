# import matplotlib.pyplot as plt
# import math
#
#
# # 从文件中读取数据
# def read_data(filename):
#     data = []
#     with open(filename, 'r') as file:
#         for line in file:
#             x, y = map(int, line.split())
#             data.append((x, y))
#
#     # 对x进行排序，并相应地调整y的顺序
#     data.sort(key=lambda pair: pair[0])
#     x_data = [pair[0] for pair in data]
#     y_data = [pair[1] for pair in data]
#
#     return x_data, y_data
#
#
# # 绘制图形
# def plot_data(x_data, y_data):
#     log_y_data = [math.log(y) if y > 0 else 0 for y in y_data]  # 处理y为0的情况
#
#     plt.plot(x_data, log_y_data, '-o', label="log(y)")
#     plt.xlabel("x")
#     plt.ylabel("log(y)")
#     plt.title("Plot of log(y) vs x")
#     plt.legend()
#     plt.grid(True)
#     plt.show()
#
#
# if __name__ == "__main__":
#     x_data, y_data = read_data("../Output/burstDuration.out")
#     plot_data(x_data, y_data)


import matplotlib.pyplot as plt


# 从文件中读取数据
def read_data(filename):
    data = []
    with open(filename, 'r') as file:
        for line in file:
            x, y = map(int, line.split())
            data.append((x, y))

    # 对x进行排序，并相应地调整y的顺序
    data.sort(key=lambda pair: pair[0])
    x_data = [pair[0] for pair in data]
    y_data = [pair[1] for pair in data]

    return x_data, y_data


# 计算y的百分比
def compute_percentage(y_data):
    total_y = sum(y_data)
    return [(y / total_y) * 100 for y in y_data]


# 绘制图形
def plot_data(x_data, y_percent_data):
    plt.plot(x_data, y_percent_data, '-o', label="y %")
    plt.xlabel("x")
    plt.ylabel("y %")
    plt.title("Plot of y % vs x")
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    x_data, y_data = read_data("../Output/burstDuration.out")
    y_percent_data = compute_percentage(y_data)
    plot_data(x_data, y_percent_data)

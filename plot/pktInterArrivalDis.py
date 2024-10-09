import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
labelFontStyle = {'family':'Arial', 'size':12, 'weight':'black'}
# 读取数据
file_path = '../output/pktInterArrivalTime.out'
inter_arrival_times = []
packet_counts = []

with open(file_path, 'r') as f:
    for line in f.readlines():
        time, count = map(int, line.strip().split())
        if time <= 30:
            inter_arrival_times.append(time)
            packet_counts.append(count)

# 计算占比
total_packets = sum(packet_counts)
proportions = [count / total_packets for count in packet_counts]

# 绘制图形
plt.bar(inter_arrival_times, proportions, color=cst.style15.color3)
plt.xlabel('Packet Inter-Arrival Time (μs)', font=labelFontStyle)
plt.ylabel('Proportion', font=labelFontStyle)
# plt.title('Packet Inter-Arrival Time Distribution')
plt.show()

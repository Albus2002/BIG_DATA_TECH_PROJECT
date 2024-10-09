import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
labelFontStyle = {'family':'STSong', 'size':14, 'weight':'heavy'}
def format_percent(pct):
    if pct < 0.5:
        return ""
    else:
        return "{:.1f}%".format(pct)

# 从文件中读取数据
with open("../output/protocolCount.out", "r") as file:
    file.readline()  # 跳过第一行标签
    total_data = file.readline().strip().split()
    ipv4_data = file.readline().strip().split()
    ipv6_data = file.readline().strip().split()

total_label, total_packets, total_bytes = total_data
ipv4_label, ipv4_packets, ipv4_bytes = ipv4_data
ipv6_label, ipv6_packets, ipv6_bytes = ipv6_data

total_packets, total_bytes = int(total_packets), int(total_bytes)
ipv4_packets, ipv4_bytes = int(ipv4_packets), int(ipv4_bytes)
ipv6_packets, ipv6_bytes = int(ipv6_packets), int(ipv6_bytes)

other_packets = total_packets - ipv4_packets - ipv6_packets
other_bytes = total_bytes - ipv4_bytes - ipv6_bytes

# 绘制包数饼状图
plt.figure(figsize=(10, 5))
plt.subplot(121)
plt.pie([ipv4_packets, ipv6_packets, other_packets], autopct=lambda pct: format_percent(pct), startangle=90, pctdistance=1.15, textprops={'weight': 'bold'}, colors=[cst.style15.color3, cst.style15.color5,  cst.style15.color7])
plt.title("(a) 包数量占比",y=-0.1, font=labelFontStyle)

# 绘制字节数饼状图
plt.subplot(122)
plt.pie([ipv4_bytes, ipv6_bytes, other_bytes], autopct=lambda pct: format_percent(pct), startangle=90, pctdistance=1.15, textprops={'weight': 'bold'}, colors=[cst.style15.color3, cst.style15.color5,  cst.style15.color7])
plt.title("(b) 字节数占比", y=-0.1, font=labelFontStyle)

plt.legend([ipv4_label, ipv6_label, 'Other'], loc="lower right")

# 显示图形
plt.show()


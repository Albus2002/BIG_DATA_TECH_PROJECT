import socket
import sys
import os
import matplotlib
import matplotlib.pyplot as plt
topkflowPath = "../../VRoutput/topkFlow.out"
topkdetailPath = "../../VRoutput/topkFlowDetail.out"
import numpy as np


def timeDiff_ms(stsec, stusec, edsec, edusec):
    return (edsec - stsec) * 1e3 + int((edusec - stusec) / 1e3)


def processTopk():
    topk = []
    with open(topkflowPath, "r") as file:
        flag = 1
        for line in file:
            if flag == 1:
                flag = 0
                continue
            srcIP, srcPort, dstIP, dstPort, protocol = map(int, line.split())
            topk.append((srcIP, srcPort, dstIP, dstPort, protocol))

    topkDetail = {}
    stt = float('inf')
    edd = float('-inf')

    with open(topkdetailPath, "r") as file:
        for ftline in file:
            ft = map(int, ftline.split())
            detail = []
            for pkt in file:
                if pkt == '---------------------------------------------------------\n':
                    break
                tmp = list(map(int, pkt.split()))
                stt = min(stt, tmp[0])
                edd = max(edd, tmp[0])
                detail.append(list(tmp))
            topkDetail[tuple(ft)] = detail

    # allBurstTopk = {} # 统计每一秒下有多少个burst
    timeStampBurst = [0 for _ in range (edd - stt + 1)]
    onOffFlowPkt = []
    onOffFlowByte = []
    burstDuration = {}
    burstPktNum = {}
    burstByte = {}

    for flow in topk:
        startPkt = topkDetail[flow][0]
        endPkt = topkDetail[flow][-1]
        st = float(startPkt[0]) + startPkt[1] / 1e6
        ed = float(endPkt[0]) + endPkt[0] / 1e6
        duration = ed - st
        pktNum = len(topkDetail[flow])
        pktSize = 0

        for pkt in topkDetail[flow]:
            # pktSize统计
            pktSize += pkt[-1]
        print("srcIP: {}, srcPort: {}, dstIP: {}, dstPort: {}, protocol: {}".format(
            socket.inet_ntoa(socket.inet_aton(str((flow[0])))), flow[1], socket.inet_ntoa(socket.inet_aton(str((flow[0])))),
            flow[3], flow[4]
        ))
        print("Duration: {}, pktNum: {}, pktSize: {}".format(duration, pktNum, pktSize))

        # burst操作
        burst = []
        for i in range(0, len(topkDetail[flow])):
            j = i
            detail = topkDetail[flow]
            byteCnt = detail[i][-1]
            while j + 1 < len(detail) and timeDiff_ms(detail[j][0], detail[j][1], detail[j + 1][0], detail[j + 1][1]) < 1:  # 1ms作为burst内包时间间隔的阈值
                j += 1
                byteCnt += detail[j][-1]
            if j == i:
                continue
            if (j - i + 1) not in burstPktNum:
                burstPktNum[j - i + 1] = 1
            else:
                burstPktNum[j - i + 1] += 1
            if byteCnt not in burstByte:
                burstByte[byteCnt] = 1
            else:
                burstByte[byteCnt] += 1
            duration = timeDiff_ms(detail[i][0], detail[i][1], detail[j][0], detail[j][1])
            if duration not in burstDuration:
                burstDuration[duration] = 1
            else:
                burstDuration[duration] += 1
            for tt in range(detail[j][0] - detail[i][0] + 1):
                relativeTime = detail[i][0] - stt + tt
                timeStampBurst[relativeTime] += 1
            i = j

        onOffPkt = [0 for _ in range(edd - stt + 1)]
        onOffByte = [0 for _ in range(edd - stt + 1)]
        for i in range(0, len(topkDetail[flow])):
            j = i
            detail = topkDetail[flow]
            while j + 1 < len(detail) and timeDiff_ms(detail[j][0], detail[j][1], detail[j + 1][0], detail[j + 1][1]) < 1000:
                onOffPkt[detail[j][0] - stt] += 1
                onOffByte[detail[j][0] - stt] += detail[j][-1]
                j += 1
            onOffPkt[detail[j][0] - stt] += 1
            onOffByte[detail[j][0] - stt] += detail[j][-1]
            i = j
        onOffFlowPkt.append(onOffPkt)
        onOffFlowByte.append(onOffByte)
    return burstDuration, burstPktNum, burstByte, onOffFlowPkt, onOffFlowByte, timeStampBurst


def plotBurstDuration(burstDuration):
    # 排序数据
    keys = sorted(burstDuration.keys())
    values = [burstDuration[key] for key in keys]

    # 创建条形图
    plt.bar(keys, values)

    # 设置标题和标签
    plt.title('Burst Duration Statistics')
    plt.xlabel('Burst Duration (ms)')
    plt.ylabel('Frequency')

    # 显示图表
    plt.show()


def plotBurstPktNum(burstPktNum):
    # 按键排序数据以便更有序地显示图表
    keys = sorted(burstPktNum.keys())
    values = [burstPktNum[key] for key in keys]

    plt.bar(keys, values)

    plt.xlabel('Number of Packets in a Burst')
    plt.ylabel('Number of Bursts')
    plt.title('Frequency of Bursts by Packet Count')
    plt.xticks(keys)  # 设置x轴刻度标签

    plt.show()


def plotBurstByte(burstByte):
    # 定义分组的跨度
    bucket_size = 1000

    # 创建一个新的分组字典
    grouped_data = {}

    # 对每个数据点进行分组
    for byte_count, num_bursts in burstByte.items():
        # 计算这个数据点应该在哪个桶
        bucket = byte_count // bucket_size * bucket_size
        # 将数据点加到正确的桶
        grouped_data[bucket] = grouped_data.get(bucket, 0) + num_bursts

    # 准备画图数据
    keys = sorted(grouped_data.keys())
    values = [grouped_data[key] for key in keys]

    # 为了美观，可以考虑添加桶的范围作为x轴标签
    labels = [f"{key}-{key + bucket_size}" for key in keys]

    plt.bar(range(len(keys)), values, tick_label=labels)

    plt.xlabel('Byte Range')
    plt.ylabel('Number of Bursts')
    plt.title('Frequency of Bursts by Byte Count Range')

    # 设置x轴刻度标签，旋转45度便于阅读
    plt.xticks(rotation=45)

    plt.tight_layout()  # 自动调整子图参数
    plt.show()


def plotBurst(timeStampBurst):
    # 移动平均的窗口大小，可以根据数据的特点调整
    window_size = 100

    # 使用NumPy的convolve函数计算移动平均
    cumsum_vec = np.cumsum(np.insert(timeStampBurst, 0, 0))
    moving_averages = (cumsum_vec[window_size:] - cumsum_vec[:-window_size]) / window_size

    # 创建平滑后的时间戳索引，注意移动平均会减少数组的长度
    smoothed_timeStamps = list(range(len(moving_averages)))

    # 绘制平滑后的折线图
    plt.plot(smoothed_timeStamps, moving_averages, label='Smoothed Burst Count')

    plt.xlabel('Time')
    plt.ylabel('Smoothed Number of Bursts')
    plt.title('Smoothed Time Series of Burst Counts')
    plt.legend()
    plt.grid(True)
    plt.show()


def plotOnOffPkt(onOffFlowPkt):
    window_size = 1  # 定义一个窗口大小用于平滑

    colors = ['b', 'g', 'r', 'c', 'm']  # 定义颜色列表

    # 绘制每条流的平滑折线图
    for i, flow in enumerate(onOffFlowPkt):
        # 计算每个流的移动平均
        cumsum_vec = np.cumsum(np.insert(flow, 0, 0))
        moving_averages = (cumsum_vec[window_size:] - cumsum_vec[:-window_size]) / window_size

        # 创建时间戳索引，注意移动平均会减少数组的长度
        smoothed_timeStamps = list(range(len(moving_averages)))

        # 绘制折线图
        plt.plot(smoothed_timeStamps, moving_averages, label=f'Flow {i + 1}', color=colors[i])

    plt.xlabel('Time')
    plt.ylabel('Smoothed Number of Packets')
    plt.title('Smoothed Time Series of Packet Counts for Multiple Flows')
    plt.legend()
    plt.grid(True)
    plt.show()


def plotOnOffByte(onOffFlowByte):
    window_size = 100  # 定义一个窗口大小用于平滑

    colors = ['b', 'g', 'r', 'c', 'm']  # 定义颜色列表

    # 绘制每条流的平滑折线图
    for i, flow in enumerate(onOffFlowByte):
        # 计算每个流的移动平均
        cumsum_vec = np.cumsum(np.insert(flow, 0, 0))
        moving_averages = (cumsum_vec[window_size:] - cumsum_vec[:-window_size]) / window_size

        # 创建时间戳索引，注意移动平均会减少数组的长度
        smoothed_timeStamps = list(range(len(moving_averages)))

        # 绘制折线图
        plt.plot(smoothed_timeStamps, moving_averages, label=f'Flow {i + 1}', color=colors[i])

    plt.xlabel('Time')
    plt.ylabel('Smoothed Number of Bytes')
    plt.title('Smoothed Time Series of Byte Counts for Multiple Flows')
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    burstDuration, burstPktNum, burstByte, onOffFlowPkt, onOffFlowByte, timeStampBurst = processTopk()
    # plotBurstDuration(burstDuration)
    # plotBurstPktNum(burstPktNum)
    # plotBurstByte(burstByte)
    # plotBurst(timeStampBurst)
    plotOnOffPkt(onOffFlowPkt)
    # plotOnOffByte(onOffFlowByte)
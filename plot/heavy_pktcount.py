import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':12, 'weight':'black'}
# Read packet sizes and corresponding counts from a specified file
file_path = "../output/heavyFlowPktSize.out"
packet_data = []
with open(file_path, "r") as file:
    for line in file:
        size, count = map(int, line.split())
        packet_data.append((size, count))

# Determine the number of intervals and width of each interval
interval_count = 50
interval_width = (10000 - 0) // interval_count

# Initialize a list to store the packet counts for each interval
packet_counts = [0] * interval_count
outrange_count = 0

# Assign packets to the corresponding intervals based on their size, and accumulate the counts
for size, count in packet_data:
    index = size // interval_width
    if index >= interval_count:
        outrange_count += count
        continue
    packet_counts[index] += count

# Calculate the total number of packets
total_packets = sum(packet_counts)

# Calculate the proportion of packets in each interval
packet_proportions = [count/total_packets for count in packet_counts]

# Generate the x-axis labels, showing a label every 300 units
xtick_labels = [i * interval_width if i * interval_width % 2000 == 0 else '' for i in range(interval_count)]

# Plot the bar chart
plt.bar(range(interval_count), packet_proportions, tick_label=xtick_labels, color = cst.style15.color3)
plt.bar(range(interval_count), [0]*(interval_count-1)+[outrange_count/total_packets], bottom=packet_proportions, tick_label=xtick_labels, color = cst.style15.color5)

# Set the axis labels and title
plt.xlabel("Flow Packet Count", font=labelFontStyle)
plt.ylabel("Flow Count", font=labelFontStyle)
# plt.title("Packet Count Distribution")

# Rotate the x-axis tick labels for better clarity
plt.xticks(rotation=45, ha="right")

# Hide x-axis ticks
plt.tick_params(axis='x', which='both', length=0)

# Display the plot
plt.show()
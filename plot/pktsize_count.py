import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
labelFontStyle = {'family':'Arial', 'size':12, 'weight':'black'}
# Read packet sizes and corresponding counts from a specified file
file_path = "../output/packetSize.out"
packet_data = []
with open(file_path, "r") as file:
    for line in file:
        size, count = map(int, line.split())
        packet_data.append((size, count))

# Determine the number of intervals and width of each interval
interval_count = 30
interval_width = (1500 - 0) // interval_count

# Initialize a list to store the packet counts for each interval
packet_counts = [0] * interval_count

# Assign packets to the corresponding intervals based on their size, and accumulate the counts
for size, count in packet_data:
    index = size // interval_width
    if index >= interval_count:
        index = interval_count - 1
    packet_counts[index] += count

# Calculate the total number of packets
total_packets = sum(packet_counts)

# Calculate the proportion of packets in each interval
packet_proportions = [count / total_packets for count in packet_counts]

# Generate the x-axis labels, showing a label every 300 units
xtick_labels = [i * interval_width if i * interval_width % 200 == 0 else '' for i in range(interval_count)]

# Plot the bar chart
plt.bar(range(interval_count), packet_proportions, tick_label=xtick_labels, color = cst.style15.color3)

# Set the axis labels and title
plt.xlabel("Packet Size", font=labelFontStyle)
plt.ylabel("Packet Proportion", font=labelFontStyle)
# plt.title("Packet Size Distribution")

# Rotate the x-axis tick labels for better clarity
plt.xticks(rotation=45, ha="right")

# Hide x-axis ticks
plt.tick_params(axis='x', which='both', length=0)

# Display the plot
plt.show()
import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

# Read packet sizes and corresponding counts from a specified file
file_path = "../output/packetSize.out"
packet_data = []
with open(file_path, "r") as file:
    for line in file:
        size, count = map(int, line.split())
        packet_data.append((size, count))

# Filter packet_data to include only packet sizes <= 100
filtered_packet_data = [data for data in packet_data if 50 <= data[0] <= 100]

# Calculate the total number of packets
total_packets = sum(count for _, count in filtered_packet_data)

# Calculate the proportion of packets for each size
packet_proportions = {size: count / total_packets for size, count in filtered_packet_data}

# Initialize an array of zeros with the length of 101
proportions_array = [0] * 51

# Fill the proportions_array with the values from packet_proportions
for size, proportion in packet_proportions.items():
    proportions_array[size-50] = proportion

# Generate the x-axis labels, showing a label every 10 units
xtick_labels = [i if i % 10 == 0 else '' for i in range(50, 101)]

# Plot the bar chart
plt.bar(range(51), proportions_array, tick_label=xtick_labels, color = cst.style01.color1)

# Set the axis labels and title
plt.xlabel("Packet Size")
plt.ylabel("Packet Proportion")
plt.title("Packet Size Distribution (50 <= Size <= 100)")

# Rotate the x-axis tick labels for better clarity
plt.xticks(rotation=45, ha="right")

# Hide x-axis ticks
plt.tick_params(axis='x', which='both', length=0)

# Display the plot
plt.show()

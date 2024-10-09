import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

# Read flow data from a specified file
file_path = "../output/fiveTupleInfo.out"
flow_data = []
with open(file_path, "r") as file:
    for line in file:
        # Split the line into three values, only use the first two values
        packet_count, total_size, _ = map(int, line.split())
        # Filter out outliers based on packet count and total size in bytes
        if packet_count <= 2500000 and total_size <= 1 * 1024:
            flow_data.append((packet_count, total_size))

# Extract packet counts and total sizes for each flow, and convert total sizes to KB
packet_counts = [x[0] for x in flow_data]
total_sizes_kb = [x[1] / 1024 for x in flow_data]

# Create the scatter plot with smaller points
plt.scatter(packet_counts, total_sizes_kb, s=5)

# Set the axis labels and title
plt.xlabel("Number of Packets in Flow")
plt.ylabel("Total Size of Packets in Flow (KB)")
# plt.title("Scatter Plot of Network Flow Analysis")

# Display the plot
plt.show()
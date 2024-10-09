import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
import math
import random
labelFontStyle = {'family':'Arial', 'size':12, 'weight':'black'}
# Read flow data from a specified file
file_path = "../output/fiveTupleInfo_sample_100.out"
flow_data = []
with open(file_path, "r") as file:
    for line in file:
        # Split the line into three values, only use the first two values
        packet_count, total_size, _ = map(int, line.split())
        rand = random.randint(0,packet_count)
        if 1/(rand+1) < 0.08:
            flow_data.append((packet_count, total_size))

# Extract packet counts and total sizes for each flow, and convert total sizes to KB
packet_counts = [math.log(x[0],10) for x in flow_data]
total_sizes_kb = [math.log(x[1], 10) for x in flow_data]

# Create the scatter plot
plt.scatter(packet_counts, total_sizes_kb, s=3, color=cst.style15.color3)

# Set the axis labels and title
plt.xlabel("Packet Count (log 10)", font=labelFontStyle)
plt.ylabel("Flow Size(Byte, 1og 10)", font=labelFontStyle)
# plt.title("Scatter Plot of Network Flow Analysis")

# Display the plot
plt.show()
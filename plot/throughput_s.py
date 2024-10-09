import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':10, 'weight':'black'}

# Read data from the file
data_mb = []
s_limit = 5000000
i = 0
with open("../output/throughput.out", "r") as f:
    for line in f:
        # Convert bytes to MB
        throughput_mb = int(line.strip())*8*1000 / (1024 * 1024 * 1024)
        data_mb.append(throughput_mb)
        i += 1
        if i >= s_limit:
            break
del data_mb[:5]
del data_mb[-5:]


# Create the x-axis (time in seconds)
time = list(range(len(data_mb)))

# Plot the data
plt.plot(time, data_mb, label="Throughput", color=cst.style15.color3)

# Fill the area under the curve with color
# plt.fill_between(time, data_mb, color=cst.style15.color3)

# Set labels and title
plt.xlabel("Time (ms)", font=labelFontStyle)
plt.ylabel("Throughput (Gbps)", font=labelFontStyle)
# plt.title("Throughput vs Time")

# Display the graph
plt.show()
import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':10, 'weight':'black'}
# Read data from the file
data_mb = []
s_limit = 12000
ts_limit = 1500
slice_ts = 300
data_group = []
colors = [cst.style15.color1, cst.style15.color2, cst.style15.color3, cst.style15.color4, cst.style15.color5, cst.style15.color6, cst.style15.color7]
i = 0
def moving_average(data, window_size):
    """Calculate the moving average for a given dataset and window size."""
    result = []
    for i in range(len(data) - window_size + 1):
        result.append(sum(data[i:i + window_size]) / window_size)
    return result

# Read throughput data from a specified file
file_path = "../output/throughput.out"
throughput_data = []
with open(file_path, "r") as file:
    for line in file:
        # Convert bytes to MB
        throughput_mb = int(line.strip())*8 / (1024 * 1024 * 1024)
        throughput_data.append(throughput_mb)
        i += 1
        if i >= s_limit:
            break
del throughput_data[:5]
del throughput_data[-5:]

# Apply moving average to smooth the data
window_size = 10
smoothed_throughput_data = moving_average(throughput_data, window_size)

for i in range(ts_limit):
    if i%slice_ts == 0:
        data_group.append([])
    data_group[i//slice_ts].append(smoothed_throughput_data[i])

# Create the x-axis (time in seconds)
time = list(range(slice_ts))

# Plot the data
for i in range(ts_limit//slice_ts):
    plt.plot(time, data_group[i], label="slice %d"%(i+1), color=colors[(2*i)%7], linewidth=2)

# Fill the area under the curve with color
# plt.fill_between(time, data_mb, color=cst.style15.color3)

# Set labels and title
plt.xlabel("Time (s)",font=labelFontStyle)
plt.ylabel("Throughput (Gbps)",font=labelFontStyle)
plt.legend()
# plt.title("Throughput vs Time")

# Display the graph
plt.show()
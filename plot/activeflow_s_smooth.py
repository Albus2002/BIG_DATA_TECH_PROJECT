import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':10, 'weight':'black'}
def moving_average(data, window_size):
    """Calculate the moving average for a given dataset and window size."""
    result = []
    for i in range(len(data) - window_size + 1):
        result.append(sum(data[i:i + window_size]) / window_size)
    return result

# Read data from the file
s_limit = 1800
i = 0
data = []
# Read data from the file
with open("../output/activeFlow.out", "r") as f:
    for line in f.readlines():
        data.append(int(line.strip()))
        i += 1
        if i >= s_limit:
            break
del data[:5]
del data[-5:]

# Convert the data to Megabytes per second

# Create the x-axis (time in seconds)
time = list(range(len(data)))

window_size = 20
smoothed_active_data = moving_average(data, window_size)

# Create the smoothed line plot
plt.plot(smoothed_active_data, color=cst.style15.color3)

# Fill the area under the curve with color
# plt.fill_between(time, data_mb, color=cst.style15.color3)

# Set labels and title
plt.xlabel("Time (s)", font=labelFontStyle)
plt.ylabel("Active Flow", font=labelFontStyle)
# plt.title("Throughput vs Time")

# Display the graph
plt.show()
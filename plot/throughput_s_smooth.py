import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':10, 'weight':'black'}

def moving_average(data, window_size):
    """Calculate the moving average for a given dataset and window size."""
    result = []
    for i in range(len(data) - window_size + 1):
        result.append(sum(data[i:i + window_size]) / window_size)
    return result

# Read throughput data from a specified file
file_path = "../output/throughput.out"
throughput_data = []
s_limit = 1800
i = 0
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
window_size = 30
smoothed_throughput_data = moving_average(throughput_data, window_size)

# Create the smoothed line plot
plt.plot(smoothed_throughput_data, color=cst.style15.color3)

# plt.fill_between(list(range(len(smoothed_throughput_data))), smoothed_throughput_data, color='skyblue', alpha=0.5)

# Set the axis labels and title
plt.xlabel("Time (s)",font=labelFontStyle)
plt.ylabel("Throughput (Gbps)", font=labelFontStyle)
# plt.title("Smoothed Throughput Over Time")

# Display the plot
plt.show()
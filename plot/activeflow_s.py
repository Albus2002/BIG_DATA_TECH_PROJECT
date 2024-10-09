import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':10, 'weight':'black'}

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
data_mb = [x for x in data]

# Create the x-axis (time in seconds)
time = list(range(len(data_mb)))

# Plot the data
plt.plot(time, data_mb, label="Active Flow", color=cst.style15.color3)

# Fill the area under the curve with color
# plt.fill_between(time, data_mb, color=cst.style15.color3)

# Set labels and title
plt.xlabel("Time (s)", font=labelFontStyle)
plt.ylabel("Active Flow", font=labelFontStyle)
# plt.title("Throughput vs Time")

# Display the graph
plt.show()
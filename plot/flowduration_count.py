import math
import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
labelFontStyle = {'family':'Arial', 'size':12, 'weight':'heavy'}

# Read data from the file
file_path = "../output/fiveTupleInfo_sample_20.out"
flow_durations = []

with open(file_path, "r") as file:
    for line in file:
        _, _, duration = line.split()
        if int(duration) != 0:
            log_duration = math.log(int(duration), 10)
            flow_durations.append(log_duration)

# Create a histogram
fig, ax = plt.subplots(figsize=(10, 6))
ax.hist(flow_durations, bins=50, edgecolor='black', color=cst.style15.color3, weights=[1./len(flow_durations)]*len(flow_durations))

# Set axis labels and title
ax.set_xlabel('Flow Duration (us, log10 scale)', font=labelFontStyle)
ax.set_ylabel('Frequency', font=labelFontStyle)
# ax.set_title('Histogram of Flow Duration (log10 scale)')

# Display the plot
plt.show()

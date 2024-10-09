import math
import matplotlib.pyplot as plt

# Read data from the file
file_path = "../output/fiveTupleInfo_sample_20.out"
flow_durations_large = []
flow_durations_small = []

with open(file_path, "r") as file:
    for line in file:
        _, byte_count, duration = line.split()
        if int(duration) > 0:
            log_duration = math.log(int(duration), 10)
            if int(byte_count) > 400:
                flow_durations_large.append(log_duration)
            else:
                flow_durations_small.append(log_duration)

# Create a histogram
fig, ax = plt.subplots(figsize=(10, 6))
ax.hist([flow_durations_small, flow_durations_large], bins=50, stacked=True, edgecolor='black',
        label=['Small Flows (<=100 KB)', 'large Flows (>100 KB)'])

# Set axis labels and title
ax.set_xlabel('Flow Duration (log10 scale)')
ax.set_ylabel('Frequency')
ax.set_title('Histogram of Flow Duration (log10 scale) - Large and Small Flows')

# Add legend
ax.legend(loc='upper left')

# Display the plot
plt.show()

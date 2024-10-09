import math
import matplotlib.pyplot as plt
import colorExample.colorStyle as cst

labelFontStyle = {'family':'Arial', 'size':12, 'weight':'black'}

# Read data from the file
file_path = "../output/fiveTupleInfo_sample_20.out"
flow_size = []

with open(file_path, "r") as file:
    for line in file:
        _, size, _  = line.split()
        if int(size) != 0:
            log_size = math.log(int(size), 10)
            flow_size.append(log_size)



# Create a histogram
fig, ax = plt.subplots(figsize=(10, 6))
ax.hist(flow_size, bins=50, edgecolor='black', color=cst.style15.color3, weights=[1./len(flow_size)]*len(flow_size))
ax.set_xticks(range(5, int(max(flow_size)), 1))
# Set axis labels and title
ax.set_xlabel('Flow Size (Byte, log10 scale)', font=labelFontStyle)
ax.set_ylabel('Frequency', font=labelFontStyle)
# ax.set_title('Histogram of Flow Duration (log10 scale)')

# Display the plot
plt.show()

import matplotlib.pyplot as plt
import math
import colorExample.colorStyle as cst
import random

labelFontStyle = {'family':'Arial', 'size':12, 'weight':'black'}
# Read data from the file
file_path = "../output/fiveTupleInfo_sample_20.out"
heavyflow_durations = []
heavyflow_pkt_counts = []
littleflow_durations = []
littleflow_pkt_counts = []
littleflow_sample_prop = 0.01

with open(file_path, "r") as file:
    for line in file:
        pkt_count, size, duration = line.split()
        # if int(pkt_count) < 2000:
        if int(duration) > 0:
            if int(size) >= 100000:
                heavyflow_durations.append(math.log(int(duration), 10))
                heavyflow_pkt_counts.append(math.log(int(pkt_count), 10))
            else:
                rand = random.random()
                # if rand < littleflow_sample_prop and math.log(int(duration)) > 4.5 and int(pkt_count) < 5000:
                #     littleflow_durations.append(math.log(int(duration), 10))
                #     littleflow_pkt_counts.append(math.log(int(pkt_count), 10))
                # elif math.log(int(duration)) <= 4.5:
                #     littleflow_durations.append(math.log(int(duration), 10))
                #     littleflow_pkt_counts.append(math.log(int(pkt_count), 10))
                # elif int(pkt_count) > 5000:
                #     littleflow_durations.append(math.log(int(duration), 10))
                #     littleflow_pkt_counts.append(math.log(int(pkt_count), 10))
                if rand < littleflow_sample_prop:
                    littleflow_durations.append(math.log(int(duration), 10))
                    littleflow_pkt_counts.append(math.log(int(pkt_count), 10))

# Create a scatter plot
fig, ax = plt.subplots(figsize=(10, 6))

ax.scatter(littleflow_durations, littleflow_pkt_counts, s=8, color=cst.style15.color3, label="little flow")
ax.scatter(heavyflow_durations, heavyflow_pkt_counts, s=8, color=cst.style15.color5, label="heavy flow")
ax.legend()

# Set axis labels and title
ax.set_xlabel('Flow Duration(log 10, us)', font=labelFontStyle)
ax.set_ylabel('Packet Count(log 10)', font=labelFontStyle)
# ax.set_title('Scatter Plot of Flow Duration and Packet Count')

# Display the plot
plt.show()

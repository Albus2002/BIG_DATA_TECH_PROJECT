import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
labelFontStyle = {'family':'STSong', 'size':14, 'weight':'heavy'}
# Read data from the file
file_path = "../output/layer5ProtoCount.out"
proto_data = []
proto_names = []
packet_proportions = []
size_proportions = []
color_set = [cst.style15.color3, cst.style15.color5, cst.style15.color7, cst.style15.color1, cst.style14.color3, cst.style15.color2, cst.style15.color4, cst.style15.color6, cst.style14.color4]

with open(file_path, "r") as file:
    for line in file:
        name, count, size = line.split()
        proto_data.append((name, int(count), int(size)))

# Calculate the total number of packets and total size
total_packets = sum(count for _, count, _ in proto_data)
total_size = sum(size for _, _, size in proto_data)

# Calculate proportions for each protocol
for name, count, size in proto_data:
    if count / total_packets > 0.001:
        packet_proportions.append(count / total_packets)
    if size / total_size > 0.001:
        size_proportions.append(size / total_size)
        proto_names.append(name)

packet_proportions.append(1-sum(packet_proportions))
size_proportions.append(1-sum(size_proportions))
proto_names.append("other")
# Extract protocol names
# proto_names = [name for name, _, _ in proto_data]

# Create subplots for packet count and size proportions
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6))

# Function to format percentage labels
def format_percent(pct):
    if pct < 1.5:
        return ""
    else:
        return "{:.1f}%".format(pct)

# Plot packet count proportions
wedges1, _, autotexts1 = ax1.pie(packet_proportions, pctdistance=1.15, textprops={'weight': 'bold'}, autopct=lambda pct: format_percent(pct), startangle=90, colors=color_set)
ax1.set_title("(a) 包数量占比",y=-0.1, font=labelFontStyle)
# ax1.legend(wedges1, proto_names, title="Protocols", loc="center left", bbox_to_anchor=(0.9, 0, 0.5, 1))

# Plot size proportions
wedges2, _, autotexts2 = ax2.pie(size_proportions, pctdistance=1.15, textprops={'weight': 'bold'},autopct=lambda pct: format_percent(pct), startangle=90, colors=color_set)
ax2.set_title("(b) 字节数占比", y=-0.1, font=labelFontStyle)
ax2.legend(wedges2, proto_names, title="Protocols", loc="lower left", bbox_to_anchor=(0.9, 0, 0.5, 1))

# Set font size for percentage labels
for autotext in autotexts1 + autotexts2:
    autotext.set_fontsize(10)

# Equal aspect ratio ensures that the pie charts are drawn as circles
ax1.axis('equal')
ax2.axis('equal')

# Display the plot
plt.show()

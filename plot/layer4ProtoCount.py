import matplotlib.pyplot as plt
import colorExample.colorStyle as cst
labelFontStyle = {'family':'STSong', 'size':12, 'weight':'heavy'}
def format_percent(pct):
    if pct < 0.1:
        return ""
    else:
        return "{:.1f}%".format(pct)

# Read protocol count data from a specified file
file_path = "../output/protocolCount.out"
protocol_counts = {}
with open(file_path, "r") as file:
    file.readline()  # Skip the first line
    for line in file:
        parts = line.strip().split()
        protocol = parts[0]
        packet_count = int(parts[1])
        byte_count = int(parts[2])
        protocol_counts[f"{protocol}-packets"] = packet_count
        protocol_counts[f"{protocol}-bytes"] = byte_count

# Calculate the other protocol data
ipv4_other_packets = protocol_counts["ipv4-packets"] - protocol_counts["ipv4-tcp-packets"] - protocol_counts["ipv4-udp-packets"]
ipv4_other_bytes = protocol_counts["ipv4-bytes"] - protocol_counts["ipv4-tcp-bytes"] - protocol_counts["ipv4-udp-bytes"]
ipv6_other_packets = protocol_counts["ipv6-packets"] - protocol_counts["ipv6-tcp-packets"] - protocol_counts["ipv6-udp-packets"]
ipv6_other_bytes = protocol_counts["ipv6-bytes"] - protocol_counts["ipv6-tcp-bytes"] - protocol_counts["ipv6-udp-bytes"]

# Data for the pie charts
ipv4_packet_data = [protocol_counts["ipv4-tcp-packets"], protocol_counts["ipv4-udp-packets"], ipv4_other_packets]
ipv4_byte_data = [protocol_counts["ipv4-tcp-bytes"], protocol_counts["ipv4-udp-bytes"], ipv4_other_bytes]
ipv6_packet_data = [protocol_counts["ipv6-tcp-packets"], protocol_counts["ipv6-udp-packets"], ipv6_other_packets]
ipv6_byte_data = [protocol_counts["ipv6-tcp-bytes"], protocol_counts["ipv6-udp-bytes"], ipv6_other_bytes]
labels = ["TCP", "UDP", "Other"]

# Create a figure with four subplots
fig, axs = plt.subplots(1, 2, figsize=(12, 6))

# Create the IPv4 packet count pie chart
# wedges1 = axs[0].pie(ipv4_packet_data, autopct=lambda pct: format_percent(pct), startangle=90, pctdistance=1.18, textprops={'weight': 'bold'}, colors=[cst.style15.color3, cst.style15.color5,  cst.style15.color7])
# axs[0].set_title("(a) 包数量占比",y=-0.1, font=labelFontStyle)
#
# # Create the IPv4 byte count pie chart
# wedges2 = axs[1].pie(ipv4_byte_data, autopct=lambda pct: format_percent(pct), startangle=90, pctdistance=1.18, textprops={'weight': 'bold'}, colors=[cst.style15.color3, cst.style15.color5,  cst.style15.color7])
# axs[1].set_title("(b) 字节数占比", y=-0.1, font=labelFontStyle)

# Create the IPv6 packet count pie chart
wedges3 = axs[0].pie(ipv6_packet_data, autopct=lambda pct: format_percent(pct), startangle=90, pctdistance=1.18, textprops={'weight': 'bold'}, colors=[cst.style15.color3, cst.style15.color5,  cst.style15.color7])
axs[0].set_title("(a) 包数量占比",y=-0.1, font=labelFontStyle)

# Create the IPv6 byte count pie chart
wedges4 = axs[1].pie(ipv6_byte_data, autopct=lambda pct: format_percent(pct), startangle=90, pctdistance=1.18, textprops={'weight': 'bold'}, colors=[cst.style15.color3, cst.style15.color5,  cst.style15.color7])
axs[1].set_title("(b) 字节数占比", y=-0.1, font=labelFontStyle)

# Create the legend
fig.legend(labels,  loc="lower right")

# Display the pie charts with the legend
plt.tight_layout()
plt.show()

# 打开文件并读取内容
with open('./dnsOutput/vpnDNS.txt', 'r') as file:
    content = file.read()

def ip_to_num(ip):
    return int(''.join([bin(int(chunk))[2:].zfill(8) for chunk in ip.split('.')]), 2)

def num_to_ip(num):
    return '.'.join([str(int(bin(num)[2:].zfill(32)[i:i+8], 2)) for i in range(0, 32, 8)])

# 将内容按行分割，然后去除空行
lines = [line.strip() for line in content.split('\n') if line.strip()]
ipset = set()
for line in lines:
    for it in line.split(','):
        ipset.add(str(ip_to_num(it)))


lines = list(ipset)
print(len(lines))

# 将所有 IP 地址用逗号分隔
ip_list = ','.join(lines)

# 将结果写入新的文件
with open('./dnsOutput/vpnoutput.txt', 'w') as output_file:
    output_file.write(ip_list)
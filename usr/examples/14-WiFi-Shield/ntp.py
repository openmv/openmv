# Simple NTP client
import time, pyb, network, usocket, ustruct, utime

SSID='' # Network SSID
KEY=''  # Network key
TIMESTAMP = 2208988800+946684800

# Init wlan module and connect to network
wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

# Create new socket
client = usocket.socket(usocket.AF_INET, usocket.SOCK_DGRAM)

# Get addr info via DNS
addr = usocket.getaddrinfo("pool.ntp.org", 123)[0][4]

# Send query
client.sendto('\x1b' + 47 * '\0', addr)
data, address = client.recvfrom(1024)

# Print time
t = ustruct.unpack(">IIIIIIIIIIII", data)[10] - TIMESTAMP
print ("Year:%d Month:%d Day:%d Time: %d:%d:%d" % (utime.localtime(t)[0:6]))

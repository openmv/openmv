# NTP Example using static IP.
#
# This example shows how to get the current time using NTP with the WiFi shield.

import network, usocket, ustruct, utime

SSID='' # Network SSID
KEY=''  # Network key

TIMESTAMP = 2208988800+946684800

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")

wlan = network.WINC()
# ifconfig must be called before connect()
wlan.ifconfig(('192.168.1.200', '255.255.255.0', '192.168.1.1', '192.168.1.1'))
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

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

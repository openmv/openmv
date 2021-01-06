# Ethernet LAN HTTP client example.
import network, usocket

PORT = 80
HOST = "www.google.com"

lan = network.LAN()
lan.active(True)
lan.ifconfig('dhcp')

# We should have a valid IP now via DHCP
print(lan.ifconfig())

# Get addr info via DNS
addr = usocket.getaddrinfo(HOST, PORT)[0][4]
print(addr)

# Create a new socket and connect to addr
client = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)
client.connect(addr)

# Set timeout
client.settimeout(3.0)

# Send HTTP request and recv response
client.send("GET / HTTP/1.1\r\nHost: %s\r\n\r\n"%(HOST))
print(client.recv(1024))

# Close socket
client.close()

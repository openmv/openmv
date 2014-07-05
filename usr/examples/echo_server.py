'''
    Simple Echo server
'''
import led,time
import wlan, socket

SSID=''         # Network SSID
KEY=''          # Network key
HOST = ''       # Use first available interface
PORT = 8000     # Arbitrary non-privileged port

led.off(led.RED)
led.off(led.BLUE)
led.on(led.GREEN)

# Init wlan module and connect to network
wlan.init()
wlan.connect(SSID, sec=wlan.WPA2, key=KEY)
led.off(led.GREEN)

# Wait for connection to be established
while (True):
    led.toggle(led.BLUE)
    time.sleep(250)
    led.toggle(led.BLUE)
    time.sleep(250)
    if wlan.connected():
        led.on(led.BLUE)
        break;

# We should have a valid IP now via DHCP
wlan.ifconfig()

# Create server socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)

# Set socket in blocking mode
s.setblocking(True)

# Bind and listen
s.bind((HOST, PORT))
s.listen(5)

print ('Waiting for connections..')
client, addr = s.accept()
print ('Connected to ' + addr[0] + ':' + str(addr[1]))

while True:
    buf = client.recv(128)
    print ('client: '+buff)
    s.send(buf)

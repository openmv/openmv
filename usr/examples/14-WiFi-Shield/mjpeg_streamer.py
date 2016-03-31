'''
    Simple MJPEG streaming server
'''
import wlan, socket
import led,time,sensor

SSID=''         # Network SSID
KEY=''          # Network key
HOST = ''       # Use first available interface
PORT = 8000     # Arbitrary non-privileged port

led.off(led.RED)
led.off(led.BLUE)
led.on(led.GREEN)

# Init sensor and set sensor parameters
sensor.reset()
sensor.set_brightness(-2)
sensor.set_contrast(1)
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.JPEG)

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

# Read request from client
data = client.recv(1024)

# Should parse client request here

# Send multipart header
client.send("HTTP/1.1 200 OK\r\n"   \
            "Server: OpenMV\r\n"    \
            "Content-Type: multipart/x-mixed-replace;boundary=openmv\r\n" \
            "Cache-Control: no-cache\r\n" \
            "Pragma: no-cache\r\n\r\n")

# Start streaming images
while (True):
    frame = sensor.snapshot()
    client.send("\r\n--openmv\r\n"  \
                "Content-Type: image/jpeg\r\n"\
                "Content-Length:"+str(frame.size())+"\r\n\r\n")
    client.send(frame)
    time.sleep(30)

client.close()

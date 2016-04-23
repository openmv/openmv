# MJPEG Streaming
#
# This example shows off how to do MJPEG streaming to a FIREFOX webrowser
# (IE and Chrome do not work). Just input your network SSID and KEY and then
# connect to the IP address/port printed out from ifconfig.

import sensor, image, time, network, usocket

SSID=''     # Network SSID
KEY=''      # Network key
HOST = ''   # Use first available interface
PORT = 8000 # Arbitrary non-privileged port

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(1)
sensor.set_saturation(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")
wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

# Create server socket
s = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)

# Bind and listen
s.bind([HOST, PORT])
s.listen(5)

# Set timeout to 1s
s.settimeout(1.0)

print ('Waiting for connections..')
client, addr = s.accept()
print ('Connected to ' + addr[0] + ':' + str(addr[1]))

# Read request from client
data = client.recv(1024)

# Should parse client request here

# Send multipart header
client.send("HTTP/1.1 200 OK\r\n" \
            "Server: OpenMV\r\n" \
            "Content-Type: multipart/x-mixed-replace;boundary=openmv\r\n" \
            "Cache-Control: no-cache\r\n" \
            "Pragma: no-cache\r\n\r\n")

# FPS clock
clock = time.clock()
# Start streaming images
while (True):
    clock.tick() # Track elapsed milliseconds between snapshots().
    frame = sensor.snapshot()
    cframe = frame.compressed(quality=90)
    client.send("\r\n--openmv\r\n" \
                "Content-Type: image/jpeg\r\n"\
                "Content-Length:"+str(cframe.size())+"\r\n\r\n")
    client.send(cframe)
    print(clock.fps())
client.close()

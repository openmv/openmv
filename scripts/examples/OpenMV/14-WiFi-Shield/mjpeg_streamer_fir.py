# MJPEG Streaming with FIR
#
# This example shows off how to do MJPEG streaming to a FIREFOX webrowser
# (IE and Chrome do not work). Just input your network SSID and KEY and then
# connect to the IP address/port printed out from ifconfig.

import sensor, image, network, usocket, fir

SSID=''     # Network SSID
KEY=''      # Network key
HOST = ''   # Use first available interface
PORT = 8000 # Arbitrary non-privileged port

# Reset sensor
sensor.reset()
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.RGB565)

# Initialize the thermal sensor
fir.init()

# Init wlan module and connect to network
print("Trying to connect... (may take a while)...")
wlan = network.WINC()
wlan.connect(SSID, key=KEY, security=wlan.WPA_PSK)

# We should have a valid IP now via DHCP
print(wlan.ifconfig())

# Create server socket
s = usocket.socket(usocket.AF_INET, usocket.SOCK_STREAM)

# Bind and listen
s.bind((HOST, PORT))
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
client.send("HTTP/1.1 200 OK\r\n"   \
            "Server: OpenMV\r\n"    \
            "Content-Type: multipart/x-mixed-replace;boundary=openmv\r\n" \
            "Cache-Control: no-cache\r\n" \
            "Pragma: no-cache\r\n\r\n")

# Start streaming images
while (True):
    image = sensor.snapshot()

    # Capture FIR data
    #   ta: Ambient temperature
    #   ir: Object temperatures (IR array)
    #   to_min: Minimum object temperature
    #   to_max: Maximum object temperature
    ta, ir, to_min, to_max = fir.read_ir()

    # Scale the image and belnd it with the framebuffer
    fir.draw_ir(image, ir)

    # Draw ambient, min and max temperatures.
    image.draw_string(0, 0, "Ta: %0.2f"%ta, color = (0xFF, 0x00, 0x00))
    image.draw_string(0, 8, "To min: %0.2f"%to_min, color = (0xFF, 0x00, 0x00))
    image.draw_string(0, 16, "To max: %0.2f"%to_max, color = (0xFF, 0x00, 0x00))

    cimage = image.compressed(quality=90)
    client.send("\r\n--openmv\r\n" \
                "Content-Type: image/jpeg\r\n"\
                "Content-Length:"+str(cimage.size())+"\r\n\r\n")
    client.send(cimage)

client.close()

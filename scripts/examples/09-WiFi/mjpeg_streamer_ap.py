# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# MJPEG Streaming AP.
#
# This example shows off how to do MJPEG streaming in AccessPoint mode.
# Chrome, Firefox and MJpegViewer App on Android have been tested.
# Connect to OPENMV_AP and use this URL: http://192.168.1.1:8080 to view the stream.

import sensor
import time
import network
import socket

SSID = "OPENMV_AP"  # Network SSID
KEY = "1234567890"  # Network key (must be 10 chars)
HOST = ""  # Use first available interface
PORT = 8080  # Arbitrary non-privileged port

# Reset sensor
sensor.reset()
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Init wlan module in AP mode.
wlan = network.WLAN(network.AP_IF)
wlan.config(ssid=SSID, key=KEY, channel=2)
wlan.active(True)

print("AP mode started. SSID: {} IP: {}".format(SSID, wlan.ifconfig()[0]))

# You can block waiting for client to connect
# print(wlan.wait_for_sta(100000))


def start_streaming(client):
    # Read request from client
    data = client.recv(1024)
    # Should parse client request here

    # Send multipart header
    client.send(
        "HTTP/1.1 200 OK\r\n"
        "Server: OpenMV\r\n"
        "Content-Type: multipart/x-mixed-replace;boundary=openmv\r\n"
        "Cache-Control: no-cache\r\n"
        "Pragma: no-cache\r\n\r\n"
    )

    # FPS clock
    clock = time.clock()

    # Start streaming images
    # NOTE: Disable IDE preview to increase streaming FPS.
    while True:
        clock.tick()  # Track elapsed milliseconds between snapshots().
        frame = sensor.snapshot()
        cframe = frame.compressed(quality=35)
        header = (
            "\r\n--openmv\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length:" + str(cframe.size()) + "\r\n\r\n"
        )
        client.sendall(header)
        client.sendall(cframe)
        print(clock.fps())


server = None

while True:
    if server is None:
        # Create server socket
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, True)
        # Bind and listen
        server.bind([HOST, PORT])
        server.listen(5)
        # Set server socket to blocking
        server.setblocking(True)

    try:
        print("Waiting for connections..")
        client, addr = server.accept()
    except OSError as e:
        server.close()
        server = None
        print("server socket error:", e)
        continue

    try:
        # set client socket timeout to 2s
        client.settimeout(5.0)
        print("Connected to " + addr[0] + ":" + str(addr[1]))
        start_streaming(client)
    except OSError as e:
        client.close()
        print("client socket error:", e)
        # sys.print_exception(e)

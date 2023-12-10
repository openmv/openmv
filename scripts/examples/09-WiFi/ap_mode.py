# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# WiFi AP Mode Example
#
# This example shows how to use WiFi in Access Point mode.
import network
import socket

SSID = "OPENMV_AP"  # Network SSID
KEY = "1234567890"  # Network key (must be 10 chars)
HOST = ""  # Use first available interface
PORT = 8080  # Arbitrary non-privileged port

# Init wlan module and connect to network
wlan = network.WLAN(network.AP_IF)
wlan.active(True)
wlan.config(essid=SSID, key=KEY, security=wlan.WEP, channel=2)
print("AP mode started. SSID: {} IP: {}".format(SSID, wlan.ifconfig()[0]))


def recvall(sock, n):
    # Helper function to recv n bytes or return None if EOF is hit
    data = bytearray()
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            raise OSError("Timeout")
        data.extend(packet)
    return data


def start_streaming(server):
    print("Waiting for connections..")
    client, addr = server.accept()

    # set client socket timeout to 5s
    client.settimeout(5.0)
    print("Connected to " + addr[0] + ":" + str(addr[1]))

    while True:
        try:
            # Read data from client
            data = recvall(client, 1024)
            # Send it back
            client.send(data)
        except OSError as e:
            print("start_streaming(): socket error: ", e)
            client.close()
            break


while True:
    try:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # Bind and listen
        server.bind([HOST, PORT])
        server.listen(1)

        # Set server socket to blocking
        server.setblocking(True)
        while True:
            start_streaming(server)
    except OSError as e:
        server.close()
        print("Server socket error: ", e)

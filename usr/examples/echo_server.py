'''
    Simple echo server
'''
import wlan
import socket
import select
import led, time

SSID=''     # Network SSID
KEY=''      # Network key
HOST = ''   # Use first available interface
PORT = 8000 # Arbitrary non-privileged port

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

while(True):
    print ('Waiting for connections..')
    client, addr = s.accept()
    print ('Connected to ' + addr[0] + ':' + str(addr[1]))

    # Set client socket non-blocking
    client.setblocking(False)

    while (True):
        rfds, wfds, xfds = select.select([client], [], [client], 1.0)
        if xfds:
            print("socket exception")
            break
        elif rfds:
            buf = client.recv(1024)
            if len(buf) == 0: # peer has shutdown
                print("socket closed")
                client.close()
                break
            print ("recv:"+str(buf))
            client.send(buf)
        elif wfds:
            print ("wfds")
        else:
            print ("timeout")

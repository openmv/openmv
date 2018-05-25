#! /usr/bin/env python
import time
import select
import socket
ADDR=('192.168.1.101', 8000)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(ADDR)
time.sleep(0.500)
s.send("HelloWorld")
time.sleep(0.500)
print (s.recv(10))
time.sleep(3)
print ("closing")
s.close()

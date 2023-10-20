# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# MJPEG Video Server
#
# This example shows off how to do MJPEG video streaming with your OpenMV Cam.
#
# Connect to the IP address/port printed out from ifconfig to view the stream via
# your web browser. Chrome, Firefox and the MJpegViewer App on Android have been tested.
#
# NOTE: After closing the connect you may need to close your browser or navigate back
# and wait a while before reconnecting. Modern web broswers try to keep the connection
# alive and cache state which may a little while to clear out (<30 seconds).

import network
import omv
import mjpeg
import sensor
import time

sensor.reset()

sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.VGA)

# Turn off the frame buffer connection to the IDE from the OpenMV Cam side.
#
# This needs to be done when manually compressing jpeg images at higher quality
# so that the OpenMV Cam does not try to stream them to the IDE using a fall back
# mechanism if the JPEG image is too large to fit in the IDE JPEG frame buffer on the OpenMV Cam.

omv.disable_fb(True)

# Setup Network Interface

network_if = network.WLAN(network.STA_IF)
network_if.active(True)
network_if.connect("your-ssid", "your-password")
while not network_if.isconnected():
    print("Trying to connect. Note this may take a while...")
    time.sleep_ms(1000)

# Setup MJPEG Server

server = mjpeg.mjpeg_server(network_if)

# For the call back functions below:
#
# `pathname` is the name of the stream resource the client wants. You can ignore this if it's not
# needed. Otherwise, you can use it to determine what image object to return. By default the path
# name will be "/".

# Track the current FPS.
clock = time.clock()


def setup_callback(pathname):
    clock.reset()
    clock.tick()
    print('Opening "%s"' % pathname)


def teardown_callback(pathname):
    print('Closing "%s"' % pathname)


server.register_setup_cb(setup_callback)
server.register_teardown_cb(teardown_callback)


# Called each time a new frame is needed.
def image_callback(pathname):
    img = sensor.snapshot()
    # Markup image and/or do various things.
    print(clock.fps())
    clock.tick()
    return img


# Stream does not return. It will call `image_callback` when it needs to get an image object to send
# to the remote client connecting to the server.

server.stream(image_callback, quality=70)

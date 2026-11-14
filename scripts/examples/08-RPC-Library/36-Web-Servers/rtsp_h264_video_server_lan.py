# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# RTSP H.264 Video Server
#
# This example shows off how to stream H.264 video over RTSP using the
# hardware video encoder. Compared to MJPEG this uses a fraction of the
# bandwidth for the same quality.
#
# You can use a program like VLC to view the video stream by connecting to
# the OpenMV Cam's IP address. E.g. ffplay -rtsp_transport tcp rtsp://<ip>

import asyncio
import network
import rtsp_h264
import csi
import time
import codec

csi0 = csi.CSI(stream=False)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)

W = csi0.width()
H = csi0.height()

# Hardware H.264 encoder: one keyframe per second at 30 fps, 1 Mbit/s target.
encoder = codec.H264Encoder(W, H, fps=30, bitrate=1000000, keyframe_interval=30)

# Setup Network Interface

network_if = network.LAN()
network_if.active(True)
network_if.ifconfig("dhcp")

# Setup RTSP Server

server = rtsp_h264.rtsp_server(network_if)

# Track the current FPS.
clock = time.clock()


def setup_callback(pathname, session):
    print('Opening "%s" in session %d' % (pathname, session))


def play_callback(pathname, session):
    global clock
    clock.reset()
    clock.tick()
    print('Playing "%s" in session %d' % (pathname, session))


def teardown_callback(pathname, session):
    print('Closing "%s" in session %d' % (pathname, session))


server.register_setup_cb(setup_callback)
server.register_play_cb(play_callback)
server.register_teardown_cb(teardown_callback)


# Called each time a new frame is needed; the server encodes it with the
# hardware encoder and packetizes the H.264 itself.
def image_callback(pathname, session):
    global csi0
    global clock
    img = csi0.snapshot()
    # Markup image and/or do various things.
    print(clock.fps())
    clock.tick()
    return img


# Stream does not return. The encoder's SPS/PPS are sent in the SDP and
# repeated at every keyframe so clients can join at any time.

asyncio.run(server.stream_h264(encoder, image_callback))

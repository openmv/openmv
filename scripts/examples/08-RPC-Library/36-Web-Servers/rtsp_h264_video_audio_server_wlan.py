# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# RTSP H.264 Video + Audio Server
#
# This example streams H.264 video from the hardware encoder AND the board
# microphone (16-bit PCM) over RTSP at the same time. The server captures
# the microphone into a preallocated ring buffer and interleaves both
# streams to the client.
#
# View with e.g.: ffplay -rtsp_transport tcp rtsp://<ip>

import asyncio
import network
import rtsp_h264
import mp4
import audio
import csi
import time
import codec

AUDIO_RATE = 16000

csi0 = csi.CSI(stream=False)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)

W = csi0.width()
H = csi0.height()

# Hardware H.264 encoder: one keyframe per second at 30 fps, 1 Mbit/s target.
encoder = codec.H264Encoder(W, H, fps=30, bitrate=1000000, keyframe_interval=30)

# The server drains the microphone ring buffer; the audio driver is
# owned here and just feeds it.
mic = mp4.MicSource(AUDIO_RATE)
audio.init(channels=1, frequency=AUDIO_RATE, gain_db=24)
audio.start_streaming(mic.callback)
mic.settle()  # wait out the mic filters' start-up pop

# Setup Network Interface

network_if = network.WLAN(network.STA_IF)
network_if.active(True)
network_if.connect("your-ssid", "your-password")
while not network_if.isconnected():
    print("Trying to connect. Note this may take a while...")
    time.sleep_ms(1000)

# Setup RTSP Server

server = rtsp_h264.rtsp_server(network_if)


def image_callback(pathname, session):
    return csi0.snapshot()


# Stream does not return. The SDP advertises both an H.264 video track and
# an L16 audio track; players that support audio (VLC, ffplay) will play
# both in sync.

asyncio.run(server.stream_h264(encoder, image_callback,
                               audio_rate=AUDIO_RATE, audio_callback=mic.read))

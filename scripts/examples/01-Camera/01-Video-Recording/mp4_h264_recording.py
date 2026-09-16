# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# H.264 MP4 Video Recording
#
# Note: You will need an SD card to run this example.
#
# Records H.264 video from the hardware encoder into a standard fragmented
# MP4 file that plays anywhere (VLC, browsers, phones). The file is written
# fragment-by-fragment, so a power loss only costs the last fragment.

import csi
import codec
import mp4
import time
import machine

FPS = 30  # The camera frame rate drives the recording cadence; the encoder
# follows the real frame times passed via timestamp_us.
RECORD_TIME = 10  # seconds

csi0 = csi.CSI(stream=False)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.framerate(FPS)

W = csi0.width()
H = csi0.height()

# Hardware H.264 encoder: one keyframe per second, 1 Mbit/s target.
encoder = codec.H264Encoder(W, H, fps=FPS, bitrate=1000000, keyframe_interval=FPS)

led = machine.LED("LED_RED")
led.on()

clock = time.clock()  # Create a clock object to track the FPS.
start = time.ticks_ms()
with mp4.Mp4("video.mp4", W, H, fps=FPS, encoder=encoder) as m:
    while time.ticks_diff(time.ticks_ms(), start) < RECORD_TIME * 1000:
        clock.tick()
        img = csi0.snapshot()
        ts = time.ticks_us()
        au = encoder.encode(img, timestamp_us=ts)
        m.write(au, timestamp_us=ts)
        print(clock.fps())

led.off()
encoder.deinit()
print("done: video.mp4")

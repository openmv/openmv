# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# H.264 MP4 Video + Audio Recording
#
# Note: You will need an SD card to run this example.
#
# Records H.264 video from the hardware encoder AND the board microphone
# (16-bit PCM) into one standard MP4 file. The recorder captures the
# microphone into a preallocated ring buffer and handles the mic start-up
# transient and end-of-recording fade-out. PCM audio plays in VLC/ffmpeg/
# QuickTime but not in the built-in Windows players or browsers - remux on
# the desktop for those: ffmpeg -i in.mp4 -c:v copy -c:a aac out.mp4

import audio
import csi
import codec
import mp4
import time
import machine

AUDIO_RATE = 16000
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

# The recorder drains the microphone ring buffer; the audio driver is
# owned here and just feeds it.
mic = mp4.MicSource(AUDIO_RATE)
audio.init(channels=1, frequency=AUDIO_RATE, gain_db=24)
audio.start_streaming(mic.callback)
mic.settle()  # wait out the mic filters' start-up pop

led = machine.LED("LED_RED")
led.on()

clock = time.clock()  # Create a clock object to track the FPS.
start = time.ticks_ms()

# audio_buffer must cover one keyframe interval of wall time (the muxer
# flushes audio at keyframes): 128 KB is 4 s at 16 kHz.
with mp4.Mp4("video_audio.mp4", W, H, fps=FPS,
             audio_rate=AUDIO_RATE, audio_buffer=131072,
             microphone=mic, encoder=encoder) as m:
    while time.ticks_diff(time.ticks_ms(), start) < RECORD_TIME * 1000:
        clock.tick()
        img = csi0.snapshot()
        ts = time.ticks_us()
        m.write(encoder.encode(img, timestamp_us=ts), timestamp_us=ts)
        m.poll_audio()
        print(clock.fps())

audio.stop_streaming()
led.off()
encoder.deinit()
print("done: video_audio.mp4")

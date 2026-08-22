# This work is licensed under the MIT license.
# Copyright (c) 2013-2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# H.264 MP4 Recording with Electronic Image Stabilization
#
# Records a stabilized H.264 MP4 using no extra hardware: the encoder is
# smaller than the sensor frame, and the crop window (roi) is moved each
# frame to cancel global motion measured by the encoder's own hardware
# motion estimation (codec.motion()). The margin between the sensor size
# and the encoder size is the maximum shake this can absorb.

import csi
import codec
import mp4
import time
from ulab import numpy as np

MARGIN = 64  # stabilization margin in pixels on each axis
FPS = 30  # The camera frame rate drives the recording cadence; the encoder
# follows the real frame times passed via timestamp_us.
RECORD_TIME = 10  # seconds

csi0 = csi.CSI(stream=False)
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.framerate(FPS)

W = csi0.width() - MARGIN * 2
H = csi0.height() - MARGIN * 2

encoder = codec.H264Encoder(W, H, fps=FPS, bitrate=1000000, keyframe_interval=FPS)

x, y = MARGIN, MARGIN  # crop window position

clock = time.clock()
start = time.ticks_ms()
with mp4.Mp4("stabilized.mp4", W, H, fps=FPS, encoder=encoder) as m:
    while time.ticks_diff(time.ticks_ms(), start) < RECORD_TIME * 1000:
        clock.tick()
        img = csi0.snapshot()

        au = encoder.encode(img, roi=(x, y, W, H), timestamp_us=time.ticks_us())
        m.write(au, timestamp_us=time.ticks_us())

        # Global motion = median of the encoder's per-macroblock motion
        # vectors, in quarter-pel units (P-frames only; keyframes do no
        # motion estimation). Move the crop window against it, decaying
        # back to center.
        if not encoder.keyframe():
            mo = encoder.motion()
            x -= int(np.median(mo.mv[:, :, 0])) // 4
            y -= int(np.median(mo.mv[:, :, 1])) // 4
        x += (MARGIN - x) // 8  # slow recenter
        y += (MARGIN - y) // 8
        x = min(max(x, 0), csi0.width() - W)
        y = min(max(y, 0), csi0.height() - H)

        print(clock.fps())

encoder.deinit()
print("done: stabilized.mp4")

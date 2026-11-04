# This work is licensed under the MIT license.
# Copyright (c) 2026 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Sensors Channel Example (ToF + Face Detection)
#
# sensor data and ToF depth maps over CBOR protocol channels.
# Uses CBORChannel from the protocol library for easy setup.

import csi
import tof
import struct
import random
from protocol import CBORChannel
import protocol
import ml
from ml.postprocessing.mediapipe import BlazeFace

# Initialize the camera.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.QVGA)

# Load built-in face detection model.
model = ml.Model(
    "/rom/blazeface_front_128.tflite",
    postprocess=BlazeFace(threshold=0.5),
)
print(model)

# Initialize ToF sensor.
tof.init()
tof_w = tof.width()
tof_h = tof.height()

# Register sensor channel.
sensors = CBORChannel()
sensors.add("temp", type="label", unit="Cel")
sensors.add("humidity", type="label", unit="%RH")
sensors_ch = protocol.register(name="sensors", backend=sensors)

# Register depth channel.
depth = CBORChannel()
depth.add("depth", type="depth", width=tof_w, height=tof_h)
depth_ch = protocol.register(name="ToF depth", backend=depth)

# Simulated sensor state.
temp = 25.0
humidity = 50.0

while True:
    # Capture frame and run face detection.
    img = csi0.snapshot()

    for r, score, keypoints in model.predict([img]):
        ml.utils.draw_predictions(img, [r], ("face",), ((0, 0, 255),), format=None)
        ml.utils.draw_keypoints(img, keypoints, color=(255, 0, 0))

    # Simulate sensor readings with random drift.
    temp += random.uniform(-0.3, 0.3)
    temp = max(15.0, min(35.0, temp))
    humidity += random.uniform(-0.5, 0.5)
    humidity = max(20.0, min(80.0, humidity))

    # Read ToF depth map. Zones without a valid target (status 5, 6 or 9)
    # carry stale distances, so send them as 0.
    try:
        depth_data, dmin, dmax, status = tof.read_depth(vflip=True, hmirror=True)
    except RuntimeError:
        continue

    depth_data = [d if s in (5, 6, 9) else 0.0 for d, s in zip(depth_data, status)]

    # Pack depth floats into binary (4 bytes per float).
    depth_bytes = struct.pack("<%df" % len(depth_data), *depth_data)

    # Update channels.
    sensors["temp"] = round(temp, 1)
    sensors["humidity"] = round(humidity, 1)
    depth["depth"] = depth_bytes

    depth_ch.send_event(0xFFFF)
    sensors_ch.send_event(0xFFFF)

    print(
        "temp=%.1f | humidity=%.1f | depth=%d bytes"
        % (temp, humidity, len(depth_bytes))
    )

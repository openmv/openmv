# FPS Test Script.
import sensor
import time

sensor.reset()  # Initialize the camera sensor.
sensor.set_framesize(sensor.QQVGA)  # or sensor.QQVGA (or others)
sensor.set_pixformat(sensor.RGB565)  # or sensor.GRAYSCALE
sensor.set_colorbar(True)  # Enable colorbars output

clock = time.clock()  # Tracks FPS.
for i in range(0, 600):
    clock.tick()  # Track elapsed milliseconds between snapshots().
    sensor.snapshot()  # Capture snapshot.

print("FPS:", clock.fps())

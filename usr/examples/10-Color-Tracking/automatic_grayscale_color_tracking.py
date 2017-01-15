# Automatic Grayscale Color Tracking Example
#
# This example shows off single color automatic grayscale color tracking using the OpenMV Cam.

import sensor, image, time
print("Letting auto algorithms run. Don't put anything in front of the camera!")

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(60)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
clock = time.clock()

print("Auto algorithms done. Hold the object you want to track in front of the camera.")
sensor.skip_frames(60)

# Capture the color thresholds for whatever was in the center of the image.
r = [(320//2)-(80//2), (240//2)-(60//2), 80, 60] # 80x60 center of QVGA.

print("Learning thresholds...")
threshold = [128, 128] # Middle grayscale values.
for i in range(60):
    img = sensor.snapshot()
    hist = img.get_histogram(roi=r)
    lo = hist.get_percentile(0.01) # Get the CDF of the histogram at the 1% range (ADJUST AS NECESSARY)!
    hi = hist.get_percentile(0.99) # Get the CDF of the histogram at the 99% range (ADJUST AS NECESSARY)!
    # Average in percentile values.
    threshold[0] = (threshold[0] + lo.value()) // 2
    threshold[1] = (threshold[1] + hi.value()) // 2
    for blob in img.find_blobs([threshold], pixels_threshold=100, area_threshold=100, merge=True, margin=10):
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())

print("Thresholds learned...")
print("Tracking colors...")

while(True):
    clock.tick()
    img = sensor.snapshot()
    for blob in img.find_blobs([threshold], pixels_threshold=100, area_threshold=100, merge=True, margin=10):
        img.draw_rectangle(blob.rect())
        img.draw_cross(blob.cx(), blob.cy())
    print(clock.fps())

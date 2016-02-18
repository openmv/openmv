import sensor, time

sensor.reset()
# Set sensor settings 
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(8)
sensor.set_contrast(2)

# Set sensor pixel format
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Capture image and set pixels
image = sensor.snapshot()
for y in range(0, 240):
    for x in range(0, 320):
        image.set_pixel([x, y], 0xFF)

time.sleep(1000)

# Switch to RGB565
sensor.set_pixformat(sensor.RGB565)

# Capture image and set pixels
image = sensor.snapshot()
for y in range(0, 240):
    for x in range(0, 320):
        image.set_pixel([x, y], (0xFF, 0x00, 0x00))

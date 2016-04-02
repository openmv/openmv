import time, sensor, image

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_brightness(0)
sensor.set_saturation(0)
sensor.set_gainceiling(16)
sensor.set_contrast(2)
sensor.set_framesize(sensor.QQVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Load template
template = image.Image("/template.pgm")

# Run template matching
while (True):
    img = sensor.snapshot()
    r = img.find_template(template, 0.75)
    if r:    
        img.draw_rectangle(r)
        time.sleep(50)

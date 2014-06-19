import sensor, time
# Set sensor contrast
sensor.set_contrast(1)
# Set sensor brightness
sensor.set_brightness(-2)
# Set sensor to pixel format
sensor.set_pixformat(sensor.GRAYSCALE)

# Load template
template = Image("0:/template.pgm")

# Run template matching
while (True):
    image = sensor.snapshot()
    r = image.find_template(template, 0.75)
    if r:    
        image.draw_rectangle(r)
        time.sleep(50)

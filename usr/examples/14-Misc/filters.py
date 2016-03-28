# Filters are image functions that process a single line at a time.
# Since filters process lines on the fly, they run at sensor speed.
# Note: Only one filter can be enabled at a time.
import time, sensor

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(3)
sensor.set_saturation(3)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QVGA)
sensor.set_pixformat(sensor.GRAYSCALE)

# Enable BW filter
sensor.set_image_filter(sensor.FILTER_BW, lower=200, upper=255)

# Enable SKIN filter (Note doesn't work very well on RGB)
#sensor.set_image_filter(sensor.FILTER_SKIN)

# FPS clock
clock = time.clock()
while (True):
	clock.tick()
  	img = sensor.snapshot()
    # Draw FPS
    # Note: Actual FPS is higher, the IDE slows down streaming.
  	img.draw_string(0, 0, "FPS:%.2f"%(clock.fps()))

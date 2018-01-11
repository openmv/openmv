# LetNet Example
import sensor, image, time

sensor.reset()                          # Reset and initialize the sensor.
sensor.set_contrast(3)
sensor.set_pixformat(sensor.GRAYSCALE)  # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.VGA)        # Set frame size to QVGA (320x240)
sensor.set_windowing((128, 128))        # Set 128x128 window.
sensor.skip_frames(time = 2000)         # Wait for settings take effect.
sensor.set_auto_gain(False)
sensor.set_auto_exposure(False)

while(True):
    img = sensor.snapshot()
    # NOTE: Uncomment to detect dark numbers on white background
    # img.invert()
    out = img.find_number(roi=(img.width()//2-14, img.height()//2-14, 28, 28))
    img.draw_rectangle((img.width()//2-15, img.height()//2-15, 30, 30))
    if out[1] > 5: # Confidence level
        print("Number: %d Confidence: %0.2f" %(out[0], out[1]))

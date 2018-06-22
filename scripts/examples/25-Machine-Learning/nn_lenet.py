# LetNet Example
import sensor, image, time, os, nn

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_contrast(3)
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)      # Set frame size to QVGA (320x240)
sensor.set_windowing((128, 128))       # Set 128x128 window.
sensor.skip_frames(time=100)
sensor.set_auto_gain(False)
sensor.set_auto_exposure(False)

# Load lenet network
net = nn.load('/lenet.network')
labels = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']

clock = time.clock()                # Create a clock object to track the FPS.
while(True):
    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.
    out = net.forward(img.copy().binary([(150, 255)], invert=True))
    max_idx = out.index(max(out))
    score = int(out[max_idx]*100)
    if (score < 70):
        score_str = "??:??%"
    else:
        score_str = "%s:%d%% "%(labels[max_idx], score)
    img.draw_string(0, 0, score_str)

    print(clock.fps())             # Note: OpenMV Cam runs about half as fast when connected
                                   # to the IDE. The FPS should increase once disconnected.

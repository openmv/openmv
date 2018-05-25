# CMSIS CNN example.
import sensor, image, time, os

sensor.reset()                          # Reset and initialize the sensor.
sensor.set_contrast(3)
sensor.set_pixformat(sensor.RGB565)     # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)       # Set frame size to QVGA (320x240)
sensor.set_windowing((200, 200))        # Set 128x128 window.
sensor.skip_frames(time = 100)          # Wait for settings take effect.
sensor.set_auto_gain(False)
sensor.set_auto_exposure(False)

labels = ['airplane', 'automobile', 'bird', 'cat', 'deer', 'dog', 'frog', 'horse', 'ship', 'truck']

clock = time.clock()    # Create a clock object to track the FPS.
while(True):
    clock.tick()        # Update the FPS clock.
    img = sensor.snapshot().lens_corr(1.6)  # Take a picture and return the image.
    out = img.classify_object()
    # print label_id:confidence
    #for i in range(0, len(out)):
    #    print("%s:%d "%(labels[i], out[i]), end="")
    max_idx = out.index(max(out))
    print("%s : %0.2f%% "%(labels[max_idx], (out[max_idx]/128)*100))

    #print(clock.fps())             # Note: OpenMV Cam runs about half as fast when connected
                                    # to the IDE. The FPS should increase once disconnected.

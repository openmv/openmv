# STM32 CUBE.AI on OpenMV MNIST Example
# See https://github.com/openmv/openmv/blob/master/src/stm32cubeai/README.MD

import sensor, image, time, nn_st

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_contrast(3)
sensor.set_brightness(0)
sensor.set_auto_gain(True)
sensor.set_auto_exposure(True)
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to Grayscale
sensor.set_framesize(sensor.QQQVGA)   # Set frame size to 80x60
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

# [CUBE.AI] Initialize the network
net = nn_st.loadnnst('network')

nn_input_sz = 28 # The NN input is 28x28

while(True):
    clock.tick()             # Update the FPS clock.
    img = sensor.snapshot()  # Take a picture and return the image.

    # Crop in the middle (avoids vignetting)
    img.crop((img.width()//2-nn_input_sz//2,
              img.height()//2-nn_input_sz//2,
              nn_input_sz,
              nn_input_sz))

    # Binarize the image 
    img.midpoint(2, bias=0.5, threshold=True, offset=5, invert=True)

    # [CUBE.AI] Run the inference
    out = net.predict(img)
    print('Network argmax output: {}'.format( out.index(max(out)) ))
    img.draw_string(0, 0, str(out.index(max(out))))
    print('FPS {}'.format(clock.fps())) # Note: OpenMV Cam runs about half as fast when connected

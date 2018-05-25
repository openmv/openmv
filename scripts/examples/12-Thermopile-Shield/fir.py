# Thermopile Shield Demo
#
# Note: To run this example you will need a Thermopile Shield for your OpenMV
#       Cam. Also, please disable JPEG mode in the IDE.
#
# The Thermopile Shield allows your OpenMV Cam to see heat!

import sensor, image, time, fir

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(0)
sensor.set_saturation(2)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)

# The following registers fine-tune the image
# sensor window to align it with the FIR sensor.
if (sensor.get_id() == sensor.OV2640):
    sensor.__write_reg(0xFF, 0x01) # switch to reg bank
    sensor.__write_reg(0x17, 0x19) # set HSTART
    sensor.__write_reg(0x18, 0x43) # set HSTOP

# Initialize the thermal sensor
fir.init()

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    # Capture an image
    image = sensor.snapshot()

    # Capture FIR data
    #   ta: Ambient temperature
    #   ir: Object temperatures (IR array)
    #   to_min: Minimum object temperature
    #   to_max: Maximum object temperature
    ta, ir, to_min, to_max = fir.read_ir()

    # Scale the image and belnd it with the framebuffer
    fir.draw_ir(image, ir)

    # Draw ambient, min and max temperatures.
    image.draw_string(0, 0, "Ta: %0.2f"%ta, color = (0xFF, 0x00, 0x00))
    image.draw_string(0, 8, "To min: %0.2f"%to_min, color = (0xFF, 0x00, 0x00))
    image.draw_string(0, 16, "To max: %0.2f"%to_max, color = (0xFF, 0x00, 0x00))

    # Print FPS.
    print(clock.fps())

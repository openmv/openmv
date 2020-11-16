# MLX90640 Overlay Demo with ir smoothing
#
# This example shows off how to overlay a smoothed heatmap onto your OpenMV Cam's
# live video output from the main camera.

import sensor, image, time, fir

IR_SCALE = 4

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.skip_frames(time = 2000)

# Initialize the thermal sensor
fir.init(type=fir.FIR_MLX90640, refresh=32) # 16Hz, 32Hz or 64Hz.

# Allocate another frame buffer for smoother video.
ir_buffer = image.Image(fir.width() * IR_SCALE, fir.height() * IR_SCALE, sensor.GRAYSCALE)

x_scale = sensor.width() / ir_buffer.width()
y_scale = sensor.height() / ir_buffer.height()

# FPS clock
clock = time.clock()

while (True):
    clock.tick()

    # Capture an image
    img = sensor.snapshot()

    # Capture FIR data
    #   ta: Ambient temperature
    #   ir: Object temperatures (IR array)
    #   to_min: Minimum object temperature
    #   to_max: Maximum object temperature
    ta, ir, to_min, to_max = fir.read_ir()

    # Create a secondary image and then blend into the frame buffer.

    # Convert FIR data to a grayscale image and scale
    fir.draw_ir(ir_buffer, ir, alpha=256)

    # Smooth the scaled image
    ir_buffer.mean(IR_SCALE-1)

    # Convert the grayscale FIR image to color using a palette and combine with camera image
    img.draw_image(ir_buffer, 0, 0, x_scale=x_scale, y_scale=y_scale, alpha=128, color_palette=sensor.PALETTE_IRONBOW)

    # Draw ambient, min and max temperatures.
    img.draw_string(8, 0, "Ta: %0.2f C" % ta, color = (255, 0, 0), mono_space = False)
    img.draw_string(8, 8, "To min: %0.2f C" % to_min, color = (255, 0, 0), mono_space = False)
    img.draw_string(8, 16, "To max: %0.2f C"% to_max, color = (255, 0, 0), mono_space = False)

    # Force high quality streaming...
    img.compress(quality=90)

    # Print FPS.
    print(clock.fps())

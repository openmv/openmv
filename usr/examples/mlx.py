import sensor, mlx, time

# Initialize the MLX module
mlx.init()

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(0)
sensor.set_saturation(2)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)

#The following registers fine-tune the window to align it with the FIR sensor.
sensor.__write_reg(0xFF, 0x01)
#HSTART/HSTOP
sensor.__write_reg(0x17, 0x19)
sensor.__write_reg(0x18, 0x43)

# FPS clock
clock = time.clock()
ta=to_min=to_max=0.0

while (True):
    clock.tick()
    # Capture an image
    image = sensor.snapshot()
    image.draw_string(0, 0, "Ta: %0.2f"%ta, (0xFF, 0x00, 0x00))
    image.draw_string(0, 5, "To min: %0.2f"%(to_min+ta), (0xFF, 0x00, 0x00))
    image.draw_string(0, 10, "To max: %0.2f"%(to_max+ta), (0xFF, 0x00, 0x00))

    # Capture an FIR image
    ta, to_min, to_max, ir = mlx.read(mlx.RAINBOW, 50, 0.90)

    # Scale the image and belnd it with the framebuffer
    ir.scale((160, 32))
    image.blend(ir, (0, int(120/2-32/2), 0.6))

    # Print FPS.
    print(clock.fps())

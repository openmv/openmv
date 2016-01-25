import sensor, mlx, time, lcd

# Initialize the MLX module
mlx.init(mlx.IR_REFRESH_64HZ)

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(0)
sensor.set_saturation(2)
sensor.set_pixformat(sensor.RGB565)

# Note: QQVGA2 is the LCD resolution. 
sensor.set_framesize(sensor.QQVGA2)

# The following registers fine-tune the image
# sensor window to align it with the FIR sensor.
sensor.__write_reg(0xFF, 0x01) # switch to reg bank
sensor.__write_reg(0x17, 0x1D) # set HSTART
sensor.__write_reg(0x18, 0x47) # set HSTOP

# Initialize LCD
lcd = lcd.LCD()
#lcd.clear(0x00)
lcd.set_backlight(True)

# FPS clock
clock = time.clock()

# Ambient temperature
ta = 0.0
# Minimum object temperature
to_min = 0.0
# Maximum object temperature
to_max = 0.0

while (True):
    clock.tick()
    # Capture an image
    image = sensor.snapshot()

    # Draw ambient, min and max temperatures.
    image.draw_string(0, 0, "Ta: %0.2f"%ta, (0xFF, 0x00, 0x00))
    image.draw_string(0, 5, "To min: %0.2f"%(to_min+ta), (0xFF, 0x00, 0x00))
    image.draw_string(0, 10, "To max: %0.2f"%(to_max+ta), (0xFF, 0x00, 0x00))

    # Capture an FIR image
    ta, to_min, to_max, ir = mlx.read_ir(mlx.RAINBOW, 80, 0.90)

    # Scale the image and belnd it with the framebuffer
    ir.scale((128, 32))
    image.blend(ir, (0, int(160/2-32/2), 0.6))

    # Display the image on the LCD
    lcd.write_image(image)

    # Print FPS.
    print(clock.fps())

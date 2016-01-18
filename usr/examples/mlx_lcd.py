import sensor, mlx, time, lcd

# Initialize the MLX module
mlx.init()

# Reset sensor
sensor.reset()

# Set sensor settings
sensor.set_contrast(1)
sensor.set_brightness(0)
sensor.set_saturation(2)
sensor.set_pixformat(sensor.RGB565)
# Note: QQVGA2 is the LCD resolution. 
sensor.set_framesize(sensor.QQVGA2)

#The following registers fine-tune the window to align it with the FIR sensor.
sensor.__write_reg(0xFF, 0x01)
#HSTART/HSTOP
sensor.__write_reg(0x17, 0x19)
sensor.__write_reg(0x18, 0x43)

# Initialize LCD
lcd = lcd.LCD()
lcd.clear(0x00)
lcd.set_backlight(True)

# FPS clock
clock = time.clock()

while (True):
    clock.tick()
    # Capture an image
    image = sensor.snapshot()

    # Capture an FIR image
    ir = mlx.read(mlx.RAINBOW)

    # Scale the image and belnd it with the framebuffer
    ir.scale((128, 32))
    image.blend(ir, (0, 64, 0.6))

    # Display the image on the LCD
    lcd.write_image(image)

    # Print FPS.
    print(clock.fps())

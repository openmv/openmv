import lcd, sensor, time
lcd.init()
lcd.clear(0x00)

sensor.reset()
sensor.set_contrast(2)
sensor.set_brightness(0)
sensor.set_saturation(2)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA2)

clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    lcd.write_image(image)
    print(clock.fps())

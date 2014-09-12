import sensor, lcd, mlx, time, led, gpio

lcd.init()
mlx.init()
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)

thermal_on = True

def switch_cb(line):
    global thermal_on
    if (line == 9):
        led.toggle(led.IR)
    if (line == 12):
        thermal_on = not thermal_on

gpio.EXTI(gpio.PB2,  switch_cb)
gpio.EXTI(gpio.PB3,  switch_cb)

clock = time.clock()
while (True):
  clock.tick()
  rgb = sensor.snapshot()
  rgb = rgb.scaled((128, 160))
  if (thermal_on):
    ir = mlx.read()
    x=ir.rainbow()
    x=ir.scale((64, 160))
    #rgb.blend(ir, (rgb.w/2-ir.w/2, rgb.h/2-ir.h/2), 0.4)
    rgb.blend(ir, (32, 0, 0.6))
  lcd.write_image(rgb)
  print(clock.fps())

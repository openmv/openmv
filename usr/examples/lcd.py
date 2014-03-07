import sensor, imlib, lcd
lcd.init()
lcd.clear(0xFF)
while (True):
  image = sensor.snapshot()
  lcd.write_image(image)

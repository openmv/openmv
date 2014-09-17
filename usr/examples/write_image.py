import sensor, time
sensor.set_framesize(sensor.VGA)
sensor.set_pixformat(sensor.JPEG)
sensor.set_quality (98)

image = sensor.snapshot()
f = open("1:/test.jpeg", "wb")
f.write(image)
f.close()

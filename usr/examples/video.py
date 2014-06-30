import sensor, avi, time

def test():
  sensor.reset()
  # Set sensor parameters
  sensor.set_brightness(-2)
  sensor.set_contrast(1)

  sensor.set_framesize(sensor.QVGA)
  sensor.set_pixformat(sensor.JPEG)

  vid = avi.AVI("1:/test1.avi", 320, 240, 15)

  start = time.ticks()
  clock = time.clock()

  while (True):
    clock.tick()
    # capture and store frame
    image = sensor.snapshot()
    vid.add_frame(image)
    # record 15 seconds
    if (time.ticks()-start) > 15000:
      break
    print(clock.fps())

  vid.flush()

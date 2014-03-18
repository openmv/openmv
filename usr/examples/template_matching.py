import sensor, imlib, time
sensor.set_pixformat(sensor.GRAYSCALE)
template = imlib.load_template("0:/minion.template")
clock = time.clock()
while (True):
    clock.tick()
    image = sensor.snapshot()
    obj = imlib.template_match(image, template, 0.7)
    if obj:
    imlib.draw_rectangle(image, obj)
    print (clock.fps())

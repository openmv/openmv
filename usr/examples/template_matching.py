from openmv import sensor, imlib
sensor.set_pixformat(sensor.GRAYSCALE)
template = imlib.load_template("0:/temp")
total_frames = 0
total_ticks = 0
t_start = ticks()
while (True):
  image = sensor.snapshot()
  t_start = ticks()
  obj = imlib.template_match(image, template, 0.7)
  total_ticks = total_ticks + (ticks()-t_start)
  total_frames = total_frames + 1
  print (total_ticks/total_frames)
  if obj:
    imlib.draw_rectangle(image, obj)

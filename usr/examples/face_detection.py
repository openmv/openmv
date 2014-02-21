from openmv import sensor, imlib
# Set sensor gainceiling
sensor.set_gainceiling(sensor.X32)
# Set sensor to grayscale
sensor.set_pixformat(sensor.GRAYSCALE)

# Load Haar Cascade
cascade = imlib.load_cascade("0:/face.bin")
print(cascade)

total_frames = 0
total_ticks = 0
while (True):
    image = sensor.snapshot()
    t_start = ticks()
    objects = imlib.detect_objects(image, cascade)
    total_ticks = total_ticks + (ticks()-t_start)
    total_frames = total_frames + 1
    for r in objects:
        imlib.draw_rectangle(image, r)
    print (total_ticks/total_frames)

from openmv import sensor, imlib
total_frames = 0
total_ticks = 0
while (True):
    # take snapshot
    image = sensor.snapshot()
    t_start = ticks()

    # detect blobs
    imlib.threshold(image, (255, 127, 127),  38)
    imlib.median(image, 1)
    blobs = imlib.count_blobs(image)

    total_ticks = total_ticks + (ticks()-t_start)
    total_frames = total_frames + 1

    # take new snapshot
    image = sensor.snapshot()
    for r in blobs:
        imlib.draw_rectangle(image, r)

    print (total_ticks/total_frames)
    print (len(blobs))

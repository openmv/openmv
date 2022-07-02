# Face Collection
#
# Use this script to gather face images for building a TensorFlow dataset. This script automatically
# zooms in the largest face in the field of view which you can then save using the data set editor.

import sensor, image, time

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

clock = time.clock()

largest_face = None
largest_face_timeout = 0

while(True):
    clock.tick()

    faces = sensor.snapshot().gamma_corr(contrast=1.5).find_features(image.HaarCascade("frontalface"))

    if faces:
        largest_face = max(faces, key = lambda f: f[2] * f[3])
        largest_face_timeout = 20

    if largest_face_timeout > 0:
        sensor.get_fb().crop(roi=largest_face)
        largest_face_timeout -= 1

    print(clock.fps())

# Face Recognition
#
# Use this script to run a TensorFlow lite image classifier on faces detected within an image.
# The classifier is free to do facial recognition, expression detection, or whatever.

import sensor, image, time, tf

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

clock = time.clock()

net = tf.load("trained.tflite", load_to_fb=True)
labels = [l.rstrip('\n') for l in open("labels.txt")]

while(True):
    clock.tick()

    # Take a picture and brighten things up for the frontal face detector.
    img = sensor.snapshot().gamma_corr(contrast=1.5)

    # Returns a list of rects (x, y, w, h) where faces are.
    faces = img.find_features(image.HaarCascade("frontalface"))

    for f in faces:

        # Classify a face and get the class scores list
        scores = net.classify(img, roi=f)[0].output()

        # Find the highest class score and lookup the label for that
        label = labels[scores.index(max(scores))]

        # Draw a box around the face
        img.draw_rectangle(f)

        # Draw the label above the face
        img.draw_string(f[0]+3, f[1]-1, label, mono_space=False)

    print(clock.fps())

# Keypoints descriptor example.
# This example shows how to save a keypoints descriptor to file. Show the camera an object
# and then run the script. The script will extract and save a keypoints descriptor and the image.
# You can use the keypoints_editor.py util to remove unwanted keypoints.
#
# NOTE: Please reset the camera after running this script to see the new file.
import sensor, time, image

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(1)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.QCIF)
sensor.set_pixformat(sensor.GRAYSCALE)

sensor.set_auto_gain(False, value=100)
sensor.skip_frames(30)

FILE_NAME = "desc"
img = sensor.snapshot()
# NOTE: See the docs for other arguments
kpts = img.find_keypoints(scale_factor=1.2)

image.save_descriptor(image.ORB, "/%s.orb"%(FILE_NAME), kpts)
img.save("/%s.pgm"%(FILE_NAME))

img.draw_keypoints(kpts)
sensor.snapshot()
time.sleep(1000)

# Keypoints descriptor example.
# This example shows how to save a keypoints descriptor to file. Show the camera an object
# and then run the script. The script will extract and save a keypoints descriptor and the image.
# You can use the keypoints_editor.py util to remove unwanted keypoints.
#
# NOTE: Please reset the camera after running this script to see the new file.
import sensor
import time
import image

# Reset sensor
sensor.reset()

# Sensor settings
sensor.set_contrast(3)
sensor.set_gainceiling(16)
sensor.set_framesize(sensor.VGA)
sensor.set_windowing((320, 240))
sensor.set_pixformat(sensor.GRAYSCALE)

sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False, value=100)

FILE_NAME = "desc"
img = sensor.snapshot()
# NOTE: See the docs for other arguments
# NOTE: By default find_keypoints returns multi-scale keypoints extracted from an image pyramid.
kpts = img.find_keypoints(max_keypoints=150, threshold=10, scale_factor=1.2)

if (kpts == None):
    raise(Exception("Couldn't find any keypoints!"))

image.save_descriptor(kpts, "/%s.orb"%(FILE_NAME))
img.save("/%s.pgm"%(FILE_NAME))

img.draw_keypoints(kpts)
sensor.snapshot()
time.sleep_ms(1000)
raise(Exception("Done! Please reset the camera"))

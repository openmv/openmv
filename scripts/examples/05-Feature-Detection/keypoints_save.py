# This work is licensed under the MIT license.
# Copyright (c) 2013-2023 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# Keypoints descriptor example.
# This example shows how to save a keypoints descriptor to file. Show the camera an object
# and then run the script. The script will extract and save a keypoints descriptor and the image.
# You can use the keypoints_editor.py util to remove unwanted keypoints.
import csi
import time
import image

csi0 = csi.CSI()
csi0.reset()
csi0.contrast(3)
csi0.gainceiling(16)
csi0.framesize(csi.VGA)
csi0.window((320, 240))
csi0.pixformat(csi.GRAYSCALE)
csi0.snapshot(time=2000)
csi0.auto_gain(False, gain_db=100)

FILE_NAME = "desc"
img = csi0.snapshot()

# NOTE: See the docs for other arguments
# NOTE: By default find_keypoints returns multi-scale keypoints extracted from an image pyramid.
kpts = img.find_keypoints(max_keypoints=150, threshold=10, scale_factor=1.2)

if kpts is None:
    raise (Exception("Couldn't find any keypoints!"))

image.save_descriptor(kpts, "%s.orb" % (FILE_NAME))
img.save("%s.pgm" % (FILE_NAME))

img.draw_keypoints(kpts)
csi0.snapshot()
time.sleep_ms(1000)

raise (Exception("Please reset the camera to see the new file."))

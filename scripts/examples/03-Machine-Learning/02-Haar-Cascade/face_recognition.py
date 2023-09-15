# Face recognition with LBP descriptors.
# See Timo Ahonen's "Face Recognition with Local Binary Patterns".
#
# Before running the example:
# 1) Download the AT&T faces database http://www.cl.cam.ac.uk/Research/DTG/attarchive/pub/data/att_faces.zip
# 2) Extract and copy the orl_faces directory to the SD card root.
#
# NOTE: This is just a PoC implementation of the paper mentioned above, it does Not work well in real life conditions.

import image

SUB = "s2"
NUM_SUBJECTS = 5
NUM_SUBJECTS_IMGS = 10

img = image.Image("orl_faces/%s/1.pgm" % (SUB)).mask_ellipse()
d0 = img.find_lbp((0, 0, img.width(), img.height()))
img = None

print("")
for s in range(1, NUM_SUBJECTS + 1):
    dist = 0
    for i in range(2, NUM_SUBJECTS_IMGS + 1):
        img = image.Image("orl_faces/s%d/%d.pgm" % (s, i)).mask_ellipse()
        d1 = img.find_lbp((0, 0, img.width(), img.height()))
        dist += image.match_descriptor(d0, d1)
    print("Average dist for subject %d: %d" % (s, dist / NUM_SUBJECTS_IMGS))

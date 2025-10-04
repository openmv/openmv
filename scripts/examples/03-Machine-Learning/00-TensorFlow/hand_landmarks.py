# This work is licensed under the MIT license.
# Copyright (c) 2013-2025 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# This example shows off Google's MediaPipe Hand Landmarks Detection model.

import csi
import time
import ml
from ml.preprocessing import Normalization
from ml.postprocessing import mediapipe_palm_detection_postprocess
from ml.postprocessing import mediapipe_hand_landmarks_postprocess

# Initialize the sensor.
csi0 = csi.CSI()
csi0.reset()
csi0.pixformat(csi.RGB565)
csi0.framesize(csi.VGA)
csi0.window((400, 400))

# Load built-in palm detection model
palm_detection = ml.Model("/rom/palm_detection_full_192.tflite")
print(palm_detection)

# Load built-in hand landmark model
hand_landmarks = ml.Model("/rom/hand_landmarks_full_224.tflite")
print(hand_landmarks)

# Create the palm detection post-processor. This post-processor dynamically
# generates anchors for the model input size which should only be done once.
palm_detection_postprocess = mediapipe_palm_detection_postprocess(threshold=0.4)

clock = time.clock()
while True:
    clock.tick()
    img = csi0.snapshot()

    # palms is a list of ((x, y, w, h), score, keypoints) tuples
    palms = palm_detection.predict([img], callback=palm_detection_postprocess)

    if palms:
        for r, score, keypoints in palms[0]:
            # rect is (x, y, w, h) - enlarge by 3x for hand landmarks model
            wider_rect = (r[0] - r[2], r[1] - r[3], r[2] * 3, r[3] * 3)
            # Operate on just the ROI of the detected palm
            n = Normalization(roi=wider_rect)

            # hands is a list of ((x, y, w, h), score, keypoints) tuples
            # index 0 (if present) is left hand
            # index 1 (if present) is right hand
            hands = hand_landmarks.predict([n(img)], callback=mediapipe_hand_landmarks_postprocess(threshold=0.4))

            # Draw bounding boxes around the detected hands and keypoints.
            for i, detections in enumerate(hands):
                for r, score, keypoints in detections:
                    ml.utils.draw_predictions(img, [r], ["right" if i else "left"], [(0, 0, 255)], format=None)
                    # keypoints is a ndarray of shape (21, 3)
                    # 0. wrist (x, y, z)
                    # 1. thumb cmc (x, y, z)
                    # 2. thumb mcp (x, y, z)
                    # 3. thumb ip (x, y, z)
                    # 4. thumb tip (x, y, z)
                    # 5. index finger mcp (x, y, z)
                    # 6. index finger pip (x, y, z)
                    # 7. index finger dip (x, y, z)
                    # 8. index finger tip (x, y, z)
                    # 9. middle finger mcp (x, y, z)
                    # 10. middle finger pip (x, y, z)
                    # 11. middle finger dip (x, y, z)
                    # 12. middle finger tip (x, y, z)
                    # 13. ring finger mcp (x, y, z)
                    # 14. ring finger pip (x, y, z)
                    # 15. ring finger dip (x, y, z)
                    # 16. ring finger tip (x, y, z)
                    # 17. pinky mcp (x, y, z)
                    # 18. pinky pip (x, y, z)
                    # 19. pinky dip (x, y, z)
                    # 20. pinky tip (x, y, z)
                    #
                    # mcp = Metacarpophalangeal Joint - the knuckle
                    # cmc = Carpometacarpal Joint - the base of the thumb
                    # pip = Proximal Interphalangeal Joint - the middle joint of the finger
                    # dip = Distal Interphalangeal Joint - the joint closest to the fingertip
                    # ip = Interphalangeal Joint - the joint in the thumb closest to the fingertip
                    # tip = Fingertip
                    for kp in keypoints.tolist():
                        img.draw_circle(int(kp[0]), int(kp[1]), 4, color=(255, 0, 0))

                    # Draw lines between hand joints.
                    lines = [(0, 1), (1, 2), (2, 3), (3, 4), (0, 5), (5, 6), (6, 7), (7, 8),
                             (5, 9), (9, 10), (10, 11), (11, 12), (9, 13), (13, 14), (14, 15), (15, 16),
                             (13, 17), (17, 18), (18, 19), (19, 20), (0, 17)]
                    for l in lines:
                        img.draw_line(int(keypoints[l[0]][0]), int(keypoints[l[0]][1]),
                                      int(keypoints[l[1]][0]), int(keypoints[l[1]][1]), color=(0, 255, 0))

    print(clock.fps(), "fps")

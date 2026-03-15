def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.preprocessing import Normalization
    from ml.postprocessing.mediapipe import BlazePalm
    from ml.postprocessing.mediapipe import HandLandmarks

    img = image.Image(data_path + "/hand.bmp", copy_to_fb=True)

    # First detect a palm.
    palm_detection = ml.Model("/rom/palm_detection_full_192.tflite", postprocess=BlazePalm(threshold=0.4))
    palms = palm_detection.predict([img])

    if len(palms) != 1:
        return False
    r, score, keypoints = palms[0]
    if r != [107, 194, 152, 152]:
        return False

    # Use the detected palm for landmarks.
    wider_rect = (r[0] - r[2], r[1] - r[3], r[2] * 3, r[3] * 3)
    n = Normalization(roi=wider_rect)

    hand_landmarks = ml.Model("/rom/hand_landmarks_full_224.tflite", postprocess=HandLandmarks(threshold=0.4))
    hands = hand_landmarks.predict([n(img)])

    if not hands:
        return False

    # Should have at least one hand with 21 keypoints.
    r, score, keypoints = hands[0][0]
    if r != [-6, 50, 319, 362]:
        return False
    if keypoints.shape != (21, 3):
        return False
    return True

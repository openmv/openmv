def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.mediapipe import MoveNet

    img = image.Image(data_path + "/person.bmp", copy_to_fb=True)

    model = ml.Model("/rom/movenet_singlepose_192.tflite", postprocess=MoveNet(threshold=0.4))
    detections = model.predict([img])

    if len(detections) != 1:
        return False

    r, score, keypoints = detections[0]

    if r != [20, 104, 350, 208]:
        return False

    return True

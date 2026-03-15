def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.mediapipe import BlazePalm

    img = image.Image(data_path + "/hand.bmp", copy_to_fb=True)
    model = ml.Model("/rom/palm_detection_full_192.tflite", postprocess=BlazePalm(threshold=0.4))
    output = model.predict([img])

    if len(output) != 1:
        return False
    rect, score, keypoints = output[0]
    if rect != [107, 194, 152, 152]:
        return False
    return True

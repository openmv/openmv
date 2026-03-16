def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.mediapipe import BlazeFace

    img = image.Image(data_path + "/faces.bmp", copy_to_fb=True)
    model = ml.Model("/rom/blazeface_front_128.tflite", postprocess=BlazeFace(threshold=0.4))
    output = model.predict([img])

    expected = [
        ([140, 14, 46, 46], 0.7783118),
        ([234, 157, 34, 34], 0.6603524),
        ([66, 97, 38, 38], 0.6350756),
        ([234, 2, 40, 40], 0.5184602),
    ]

    if len(output) != len(expected):
        return False
    for i, (rect, score, _kp) in enumerate(output):
        if rect != expected[i][0]:
            return False
    return True

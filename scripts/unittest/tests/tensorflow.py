def unittest(data_path, temp_path):
    import omv
    import ml
    import image

    if "MPS3" in omv.arch():
        from ml.postprocessing.mediapipe import BlazeFace

        img = image.Image(data_path + "/faces.bmp", copy_to_fb=True)
        model = ml.Model("data/blazeface_front_128.tflite", postprocess=BlazeFace(threshold=0.4))
        output = model.predict([img])

        expected_rects = [
            ([140, 14, 46, 46], 0.7783118),
            ([234, 157, 34, 34], 0.6603524),
            ([66, 97, 38, 38], 0.6350756),
            ([234, 2, 40, 40], 0.5184602),
        ]
        if len(output) != 4:
            return False
        for i, (rect, score, _kp) in enumerate(output):
            if rect != expected_rects[i][0]:
                return False
        return True
    else:
        from ml.postprocessing.edgeimpulse import Fomo

        img = image.Image(data_path + "/faces.bmp", copy_to_fb=True)
        model = ml.Model(data_path + "/fomo_face_detection.tflite", postprocess=Fomo(threshold=0.4))
        output = model.predict([img], callback=Fomo())

        return output[1] == [
            ([149, 17, 31, 31], 0.6796876),
            ([194, 198, 31, 31], 0.6289064),
            ([64, 17, 25, 31], 0.4453126),
        ]

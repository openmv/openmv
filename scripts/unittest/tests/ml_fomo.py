def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.edgeimpulse import Fomo

    img = image.Image(data_path + "/faces.bmp", copy_to_fb=True)
    model = ml.Model("/rom/fomo_face_detection.tflite", postprocess=Fomo(threshold=0.4))
    output = model.predict([img])

    return output[1] == [
        ([149, 17, 31, 31], 0.6796876),
        ([194, 198, 31, 31], 0.6289064),
        ([64, 17, 25, 31], 0.4453126),
    ]

def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.ultralytics import YoloV8

    img = image.Image(data_path + "/person.bmp", copy_to_fb=True)
    model = ml.Model("/rom/yolov8n_192.tflite", postprocess=YoloV8(threshold=0.4))
    output = model.predict([img])

    # Should detect person (class 0) with high confidence.
    if len(output[0]) != 1:
        return False
    rect, score = output[0][0]
    if score < 0.9:
        return False
    return True

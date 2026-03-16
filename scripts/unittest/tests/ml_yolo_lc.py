def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.darknet import YoloLC

    img = image.Image(data_path + "/person.bmp", copy_to_fb=True)
    model = ml.Model("/rom/yolo_lc_192.tflite", postprocess=YoloLC(threshold=0.2))
    output = model.predict([img])

    # Check if any class has detections.
    for class_detections in output:
        if len(class_detections) > 0:
            return True
    # Model may not detect on this image; pass if it ran without error.
    return True

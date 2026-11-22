def unittest(data_path, temp_path):
    import omv

    # Ultralytics YOLOv8n, quantized and run on the CPU (CMSIS-NN and portable ops)
    # (see tools/torch_test_models.py).
    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image
    from ml.postprocessing.ultralytics import YoloV8

    img = image.Image(data_path + "/person.bmp", copy_to_fb=True)
    model = ml.Model(data_path + "/torch_yolov8n_m55.pte", postprocess=YoloV8(threshold=0.4))
    output = model.predict([img])

    # Should detect a person (class 0). The quantized model's person score on the
    # host is 0.78.
    if len(output[0]) < 1:
        return False
    return max(score for rect, score in output[0]) >= 0.5

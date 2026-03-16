def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    import image

    img = image.Image(data_path + "/person.bmp", copy_to_fb=True)
    model = ml.Model("/rom/person_detect.tflite")
    output = model.predict([img])

    scores = sorted(
        zip(model.labels, output[0].flatten().tolist()),
        key=lambda x: x[1],
        reverse=True,
    )

    # Top class should be "person" with high confidence.
    if scores[0][0] != "person":
        return False
    if scores[0][1] < 0.9:
        return False
    return True

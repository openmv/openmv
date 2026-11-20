def unittest(data_path, temp_path):
    import omv

    # Model with two inputs (an NCHW image and a vector) and three outputs (float
    # scores, and their argmax as int64 and int32), see tools/torch_test_models.py.
    if "MPS2" not in omv.arch():
        return "skip"

    import ml
    import image
    from ulab import numpy as np

    model = ml.Model(data_path + "/torch_multi_io_m7.pte")

    if model.input_shape != ((1, 3, 8, 8), (1, 3)) or model.input_layout != ("NCHW", None):
        print("inputs:", model.input_shape, model.input_layout)
        return False
    if model.output_shape != ((1, 3), (1,), (1,)) or model.output_dtype != ("f", "q", "i"):
        print("outputs:", model.output_shape, model.output_dtype)
        return False

    # Wrong number of inputs.
    for inputs in ([np.zeros((1, 3))], [np.zeros((1, 3)), np.zeros((1, 3)), np.zeros((1, 3))]):
        try:
            model.predict(inputs)
            print("predict accepted %d inputs" % len(inputs))
            return False
        except ValueError:
            pass

    # The NCHW version of the NHWC test input used by the other tests.
    x = np.array([(((i % 64) * 3 + i // 64) * 37 % 255) / 127.5 - 1.0 for i in range(3 * 8 * 8)],
                 dtype=np.float).reshape((1, 3, 8, 8))
    y = np.array([[0.25, -0.5, 0.75]], dtype=np.float)

    output = model.predict([x, y])
    scores = output[0].flatten()
    expected = [2.2078, 1.4681, 2.7427]
    if max([abs(float(scores[i]) - expected[i]) for i in range(3)]) > 0.05:
        print("scores:", scores)
        return False
    if int(output[1][0]) != 2 or int(output[2][0]) != 2:
        print("argmax:", output[1], output[2])
        return False

    # Image input, normalized into the NCHW tensor. A single blue pixel makes the blue
    # channel's max 1.0, while the other channels are 0. With an NHWC (interleaved)
    # tensor, the pixel would land in the red plane instead. The quantized output
    # range is calibrated on positive values, so the -0.5 clamps to 0.
    img = image.Image(8, 8, image.RGB565)
    img.set_pixel((0, 1), (0, 0, 255))
    scores = model.predict([img, y])[0].flatten()
    expected = [0.2504, 0.0, 2.7427]
    if max([abs(float(scores[i]) - expected[i]) for i in range(3)]) > 0.05:
        print("image scores:", scores)
        return False
    return True

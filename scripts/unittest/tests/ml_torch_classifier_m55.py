def unittest(data_path, temp_path):
    import omv

    # Quantized classifier lowered to CMSIS-NN ops (exported per CPU, see
    # tools/torch_test_models.py).
    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    from ulab import numpy as np

    model = ml.Model(data_path + "/torch_classifier_m55.pte")

    if model.input_shape[0] != (1, 32, 32, 3) or model.output_shape[0] != (1, 10):
        return False

    i = np.array([((x * 37) % 255) / 127.5 - 1.0 for x in range(32 * 32 * 3)],
                 dtype=np.float).reshape(model.input_shape[0])

    output = model.predict([i])[0].flatten()
    expected = [0.0039, 0.0977, 0.0195, 0.4258, 0.0195, 0.0117, 0.0, 0.3633, 0.0156, 0.043]

    if int(np.argmax(output)) != 3:
        return False

    return max([abs(float(output[x]) - expected[x]) for x in range(10)]) < 0.02

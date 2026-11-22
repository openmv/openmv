def unittest(data_path, temp_path):
    import omv

    # Quantized detector head with CMSIS-NN ops and the portable cat and upsample
    # ops (see tools/torch_test_models.py).
    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    from ulab import numpy as np

    model = ml.Model(data_path + "/torch_detector_m55.pte")

    if model.input_shape[0] != (1, 32, 32, 3) or model.output_shape[0] != (1, 2, 8, 8):
        return False

    i = np.array([((x * 37) % 255) / 127.5 - 1.0 for x in range(32 * 32 * 3)],
                 dtype=np.float).reshape(model.input_shape[0])

    output = model.predict([i])[0].flatten()

    # Expected values from the quantized model on the host.
    return (int(np.argmax(output)) == 61 and
            abs(float(np.max(output)) - 0.6728) < 0.02 and
            abs(float(np.sum(output)) - 70.3499) < 0.5)

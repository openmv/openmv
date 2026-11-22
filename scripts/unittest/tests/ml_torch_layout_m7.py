def unittest(data_path, temp_path):
    import omv

    # Portable split, cat, slice and flip ops on channels-last tensors, between
    # CMSIS-NN convolutions (see tools/torch_test_models.py).
    if "MPS2" not in omv.arch():
        return "skip"

    import ml
    from ulab import numpy as np

    model = ml.Model(data_path + "/torch_layout_m7.pte")

    if model.input_shape[0] != (1, 16, 16, 3) or model.output_shape[0] != (1, 12, 14, 4):
        return False

    i = np.array([((x * 37) % 255) / 127.5 - 1.0 for x in range(16 * 16 * 3)],
                 dtype=np.float).reshape(model.input_shape[0])

    output = model.predict([i])[0].flatten()

    # Expected values from the quantized model on the host.
    return (int(np.argmax(output)) == 438 and
            abs(float(np.max(output)) - 0.3888) < 0.02 and
            abs(float(np.sum(output)) - 21.2021) < 0.5)

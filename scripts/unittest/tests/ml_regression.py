def unittest(data_path, temp_path):
    import omv

    if "MPS3" not in omv.arch():
        return "skip"

    import ml
    from ulab import numpy as np

    model = ml.Model("/rom/force_int_quant.tflite")

    i = np.array([-3, -1, -2, 5, -2, 10, -1, 9, 0,
                   2,  0,  9, 1, 10,  2, -1, 3, 5,
                   3,  9,  3, 9,  6,  2,  6, 7, 5,
                  10,  6, -1, 7,  4,  7,  8, 5, 7],
                  dtype=np.int8).reshape(model.input_shape[0])

    output = model.predict([i])[0]
    result = float(output.flatten()[0])
    return abs(result - 53.78332) < 0.01

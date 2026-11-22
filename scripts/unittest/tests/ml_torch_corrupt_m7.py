def unittest(data_path, temp_path):
    import omv

    # Programs with a valid header and a corrupted body must fail to load cleanly,
    # and leave the backend usable.
    if "MPS2" not in omv.arch():
        return "skip"

    import ml
    from ulab import numpy as np

    path = data_path + "/torch_classifier_m7.pte"
    with open(path, "rb") as f:
        data = bytearray(f.read())

    # Corrupt the flatbuffer after the header ("ET12" at offset 4) with different
    # patterns and strides, and truncate it.
    files = []
    for seed in range(6):
        d = bytearray(data)
        for i in range(16 + seed, min(len(d), 2048), 7 + seed):
            d[i] ^= 0x5A + seed
        files.append(d)
    files.append(data[:len(data) // 2])

    tmp = temp_path + "/torch_corrupt.pte"
    for d in files:
        with open(tmp, "wb") as f:
            f.write(d)
        try:
            ml.Model(tmp)
        except Exception:
            pass

    # The original model must still load and run.
    model = ml.Model(path)
    i = np.array([((x * 37) % 255) / 127.5 - 1.0 for x in range(32 * 32 * 3)],
                 dtype=np.float).reshape(model.input_shape[0])
    return int(np.argmax(model.predict([i])[0].flatten())) == 3

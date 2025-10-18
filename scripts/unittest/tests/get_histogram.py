def unittest(data_path, temp_path):
    import image

    e = 0.0000001
    # Load image
    img = image.Image(data_path + "/cat.pgm", copy_to_fb=True)

    # Load histogram
    with open(data_path + "/cat.csv", "r") as f:
        hist1 = [float(x) for x in f.read().split(",")]

    # Get histogram
    hist2 = img.get_histogram()

    # Compare
    for a, b in zip(hist1, hist2[0]):
        if abs(a - b) > e:
            return False

    return hist2.get_percentile(0.5)[0] == 96 and hist2.get_statistics()[0:] == (
        81,
        96,
        0,
        59,
        0,
        255,
        13,
        128,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    )

def unittest(data_path, temp_path):
    import image

    img = image.Image(data_path + "/datamatrix.pgm", copy_to_fb=True)
    matrices = img.find_datamatrices()
    return len(matrices) == 1 and matrices[0][0:] == (
        34,
        15,
        90,
        89,
        "https://openmv.io/",
        0.0,
        18,
        18,
        18,
        0,
    )

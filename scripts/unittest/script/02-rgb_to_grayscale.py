def unittest(data_path, temp_path):
    import image
    gs = image.rgb_to_grayscale((120, 200, 120))
    return  (abs(167-gs) < 2)

def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/eye.pgm", copy_to_fb=True)
    iris = img.find_eye((100, 70, 250, 100))
    return iris == (159, 114)

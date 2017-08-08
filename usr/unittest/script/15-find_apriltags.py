def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/apriltags.pgm", copy_to_fb=True)
    tags = img.find_apriltags()
    return len(tags) == 1 and tags[0][0:8] == (45, 27, 69, 69, 255, 16, 80, 61)

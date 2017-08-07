def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/shapes.ppm", copy_to_fb=True)
    rects = img.find_rects(threshold = 50000)
    return len(rects) == 1 and rects[0][0:] == (23, 39, 35, 36, 146566)

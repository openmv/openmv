def unittest(data_path, temp_path):
    import image

    img = image.Image(data_path + "/shapes.ppm", copy_to_fb=True)
    circles = img.find_circles(threshold=5000, x_margin=30, y_margin=30, r_margin=30)
    return len(circles) == 1 and circles[0][0:] == (118, 56, 22, 5856)

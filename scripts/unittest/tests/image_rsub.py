def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(40, 40, image.GRAYSCALE)

    # Fill with value 100
    for y in range(40):
        for x in range(40):
            img.set_pixel(x, y, 100)

    return True

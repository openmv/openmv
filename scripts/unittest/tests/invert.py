def unittest(data_path, temp_path):
    import image

    # Test grayscale invert
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with known values
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 100)

    # Invert the image
    img.invert()

    # Verify all pixels are inverted (255 - 100 = 155)
    for y in range(50):
        for x in range(50):
            pixel = img.get_pixel(x, y)
            if pixel != 155:
                return False

    # Test with another value
    img2 = image.Image(50, 50, image.GRAYSCALE)
    for y in range(50):
        for x in range(50):
            img2.set_pixel(x, y, 200)

    img2.invert()

    stats = img2.get_statistics()
    # 255 - 200 = 55
    if stats.mean() != 55 or stats.min() != 55 or stats.max() != 55:
        return False

    return True

def unittest(data_path, temp_path):
    import image

    # Test grayscale binary thresholding
    img_gray = image.Image(100, 100, image.GRAYSCALE)

    # Fill with gradient pattern
    for y in range(100):
        for x in range(100):
            img_gray.set_pixel(x, y, x + y)

    # Apply binary threshold at 128
    img_gray.binary([(128, 255)])

    # Verify all pixels are either 0 or 255
    for y in range(100):
        for x in range(100):
            pixel = img_gray.get_pixel(x, y)
            if pixel != 0 and pixel != 255:
                return False

    # Verify we have both black and white pixels
    stats = img_gray.get_statistics()
    if stats.min() != 0 or stats.max() != 255:
        return False

    return True

def unittest(data_path, temp_path):
    import image

    # Create image with known pixel distribution
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with uniform value 128
    for y in range(100):
        for x in range(100):
            img.set_pixel(x, y, 128)

    stats = img.get_statistics()

    # Check mean, min, max for uniform image
    if stats.mean() != 128 or stats.min() != 128 or stats.max() != 128:
        return False

    # Standard deviation should be 0 for uniform image
    if stats.stdev() != 0:
        return False

    # Create image with known distribution: half 0, half 255
    img2 = image.Image(100, 100, image.GRAYSCALE)

    for y in range(100):
        for x in range(100):
            if x < 50:
                img2.set_pixel(x, y, 0)
            else:
                img2.set_pixel(x, y, 255)

    stats2 = img2.get_statistics()

    # Mean should be 127 or 128 (approximately middle)
    if stats2.mean() < 127 or stats2.mean() > 128:
        return False

    # Min and max should be 0 and 255
    if stats2.min() != 0 or stats2.max() != 255:
        return False

    return True

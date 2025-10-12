def unittest(data_path, temp_path):
    import image
    import os

    # Create test image
    img = image.Image(60, 60, image.RGB565)

    # Fill with pattern
    for y in range(60):
        for x in range(60):
            color = ((x * 8) << 8) | ((y * 4) << 3) | ((x + y) >> 3)
            img.set_pixel(x, y, color)

    # Test BMP save/load
    bmp_path = temp_path + "/test_image.bmp"
    img.save(bmp_path)

    img_loaded = image.Image(bmp_path)
    if img_loaded.width() != 60 or img_loaded.height() != 60:
        return False

    # Cleanup
    os.remove(bmp_path)

    # Test PPM save/load
    ppm_path = temp_path + "/test_image.ppm"
    img.save(ppm_path)

    img_loaded2 = image.Image(ppm_path)
    if img_loaded2.width() != 60 or img_loaded2.height() != 60:
        return False

    # Cleanup
    os.remove(ppm_path)

    return True

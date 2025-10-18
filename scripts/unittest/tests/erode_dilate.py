def unittest(data_path, temp_path):
    import image

    # Create a simple binary image with white square in center
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with black
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 0)

    # Create white square in center (20x20)
    for y in range(15, 35):
        for x in range(15, 35):
            img.set_pixel(x, y, 255)

    # Test erode - should shrink white region
    img_erode = img.copy()
    img_erode.erode(1)

    stats_orig = img.get_statistics()
    stats_erode = img_erode.get_statistics()

    # After erosion, should have less white (lower mean)
    if stats_erode.mean() >= stats_orig.mean():
        return False

    # Test dilate - should expand white region
    img_dilate = img.copy()
    img_dilate.dilate(1)

    stats_dilate = img_dilate.get_statistics()

    # After dilation, should have more white (higher mean)
    if stats_dilate.mean() <= stats_orig.mean():
        return False

    # Test open and close complete successfully
    img_open = img.copy()
    img_open.open(1)

    img_close = img.copy()
    img_close.close(1)

    return True

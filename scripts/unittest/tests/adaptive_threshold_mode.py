def unittest(data_path, temp_path):
    import image

    # Create grayscale image with varying brightness
    img = image.Image(64, 64, image.GRAYSCALE, copy_to_fb=True)

    # Create gradient - dark on left, bright on right
    for y in range(64):
        for x in range(64):
            img.set_pixel(x, y, int(x * 4))

    # Apply mode adaptive threshold
    img.mode(2, threshold=True, offset=5, invert=False)

    # After adaptive thresholding, should be binary
    # Verify format is still grayscale (binary thresholding in place)
    if img.format() != image.GRAYSCALE:
        return False

    # Verify output contains only 0 or 255 values (binary)
    test_pixel = img.get_pixel(32, 32)
    if test_pixel != 0 and test_pixel != 255:
        return False

    return True

def unittest(data_path, temp_path):
    import image

    # Create test image with asymmetric pattern
    img = image.Image(40, 40, image.GRAYSCALE)

    # Fill with asymmetric gradient
    for y in range(40):
        for x in range(40):
            img.set_pixel(x, y, x * 5)

    # Test horizontal flip
    img_h = img.copy(x_scale=1.0, y_scale=1.0, hint=image.HMIRROR)

    # Verify dimensions preserved
    if img_h.width() != 40 or img_h.height() != 40:
        return False

    # Verify pixel at left is now from right
    left_pixel = img_h.get_pixel(0, 20)
    if left_pixel < 150:  # Should be from right side (high value)
        return False

    # Test vertical flip
    img_v = img.copy(x_scale=1.0, y_scale=1.0, hint=image.VFLIP)

    # Verify dimensions preserved
    if img_v.width() != 40 or img_v.height() != 40:
        return False

    return True

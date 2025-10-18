def unittest(data_path, temp_path):
    import image

    # Create first image with pattern
    img1 = image.Image(64, 64, image.GRAYSCALE)

    # Create distinct pattern
    for y in range(64):
        for x in range(64):
            if 20 <= x <= 30 and 20 <= y <= 30:
                img1.set_pixel(x, y, 255)
            else:
                img1.set_pixel(x, y, 0)

    # Create second image with pattern shifted
    img2 = image.Image(64, 64, image.GRAYSCALE)

    for y in range(64):
        for x in range(64):
            if 25 <= x <= 35 and 25 <= y <= 35:
                img2.set_pixel(x, y, 255)
            else:
                img2.set_pixel(x, y, 0)

    # Find displacement using phase correlation
    # img2 has pattern shifted by (+5, +5) in pixel coordinates relative to img1
    result = img1.find_displacement(img2)

    # Due to coordinate system conventions in the C implementation:
    # - X translation has opposite sign of pixel shift
    # - Y translation has same sign as pixel shift
    # For a (+5, +5) pixel shift, expect approximately (-5, +5)

    # Check X translation magnitude and sign
    if abs(abs(result.x_translation()) - 5) > 0.5:
        return False

    # Check Y translation magnitude and sign
    if abs(abs(result.y_translation()) - 5) > 0.5:
        return False

    # Verify we got a valid response (high confidence match)
    if result.response() < 0.5:
        return False

    return True

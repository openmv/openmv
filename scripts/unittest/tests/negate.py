def unittest(data_path, temp_path):
    import image

    # Test negate on grayscale image
    img = image.Image(50, 50, image.GRAYSCALE, copy_to_fb=True)

    # Fill with known values
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 100)

    # Apply negate
    img.negate()

    # Verify pixels are inverted (255 - 100 = 155)
    pixel = img.get_pixel(25, 25)
    if pixel != 155:
        return False

    # Test negate on RGB565 image
    img2 = image.Image(50, 50, image.RGB565, copy_to_fb=True)

    # Fill with known color
    for y in range(50):
        for x in range(50):
            img2.set_pixel(x, y, (100, 100, 100))

    # Apply negate
    img2.negate()

    # Get negated pixel
    r, g, b = img2.get_pixel(25, 25)

    # Verify RGB components are inverted (approximately)
    # Allow tolerance for RGB565 conversion losses
    if abs(r - 155) > 10 or abs(g - 155) > 10 or abs(b - 155) > 10:
        return False

    return True

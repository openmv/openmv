def unittest(data_path, temp_path):
    import image

    # Create binary image with noise
    img = image.Image(64, 64, image.GRAYSCALE, copy_to_fb=True)

    # Fill with black
    for y in range(64):
        for x in range(64):
            img.set_pixel(x, y, 0)

    # Create a large white region
    for y in range(20, 40):
        for x in range(20, 40):
            img.set_pixel(x, y, 255)

    # Add small noise pixels (should be removed by opening)
    img.set_pixel(5, 5, 255)
    img.set_pixel(10, 10, 255)
    img.set_pixel(50, 50, 255)

    # Convert to binary
    img.binary([(128, 255)])

    # Apply morphological opening (removes small objects)
    img.open(2, threshold=0)

    # Verify large region still exists
    center_pixel = img.get_pixel(30, 30)
    if center_pixel == 0:
        return False

    # Verify noise pixels were removed
    noise_pixel1 = img.get_pixel(5, 5)
    noise_pixel2 = img.get_pixel(10, 10)
    if noise_pixel1 != 0 or noise_pixel2 != 0:
        return False

    return True

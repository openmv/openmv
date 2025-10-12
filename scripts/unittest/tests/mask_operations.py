def unittest(data_path, temp_path):
    import image

    # Create main image
    img = image.Image(50, 50, image.GRAYSCALE)
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 150)

    # Create mask (binary image)
    mask = image.Image(50, 50, image.BINARY)
    for y in range(50):
        for x in range(50):
            if 10 <= x <= 40 and 10 <= y <= 40:
                mask.set_pixel(x, y, 1)
            else:
                mask.set_pixel(x, y, 0)

    # Apply operation with mask (e.g., set masked region to 0)
    img.binary([(0, 255)], invert=False, zero=True, mask=mask)

    # Verify masked region was affected
    center_pixel = img.get_pixel(25, 25)
    corner_pixel = img.get_pixel(5, 5)

    # Center should be modified, corner should remain
    if center_pixel == corner_pixel:
        return False

    return True

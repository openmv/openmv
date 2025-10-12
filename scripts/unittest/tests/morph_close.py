def unittest(data_path, temp_path):
    import image

    # Create binary image with a white region that has black holes
    img = image.Image(64, 64, image.GRAYSCALE, copy_to_fb=True)

    # Fill with black
    for y in range(64):
        for x in range(64):
            img.set_pixel(x, y, 0)

    # Create white region
    for y in range(20, 40):
        for x in range(20, 40):
            img.set_pixel(x, y, 255)

    # Add small black holes inside white region (should be filled by closing)
    img.set_pixel(25, 25, 0)
    img.set_pixel(30, 30, 0)
    img.set_pixel(35, 35, 0)

    # Convert to binary
    img.binary([(128, 255)])

    # Apply morphological closing (fills small black holes in white regions)
    img.close(2, threshold=0)

    # Verify white region still exists
    region_pixel = img.get_pixel(22, 22)
    if region_pixel != 255:
        return False

    # Verify small holes were filled (should now be white/255)
    hole_pixel1 = img.get_pixel(25, 25)
    hole_pixel2 = img.get_pixel(30, 30)
    if hole_pixel1 != 255 or hole_pixel2 != 255:
        return False

    return True

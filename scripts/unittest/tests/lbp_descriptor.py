def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(64, 64, image.GRAYSCALE)

    # Fill with texture pattern
    for y in range(64):
        for x in range(64):
            img.set_pixel(x, y, ((x ^ y) * 3) % 256)

    # Find LBP descriptor
    lbp = img.find_lbp((0, 0, 64, 64))

    # Verify LBP descriptor was created
    if lbp is None:
        return False

    # LBP should have descriptor data
    return True

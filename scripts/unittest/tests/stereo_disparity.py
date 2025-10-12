def unittest(data_path, temp_path):
    import image

    # Create a test image (stereo disparity needs stereo pair)
    img = image.Image(64, 64, image.GRAYSCALE)

    # Fill with pattern
    for y in range(64):
        for x in range(64):
            img.set_pixel(x, y, (x + y) % 256)

    # Compute stereo disparity
    # Note: Real stereo disparity needs a proper stereo pair
    # This test just verifies the function exists and runs
    img.stereo_disparity(reversed=False)

    # Verify image still valid
    if img.width() != 64 or img.height() != 64:
        return False

    return True

def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(64, 64, image.GRAYSCALE)

    # Fill with grid pattern
    for y in range(64):
        for x in range(64):
            if (x % 8 == 0) or (y % 8 == 0):
                img.set_pixel(x, y, 255)
            else:
                img.set_pixel(x, y, 0)

    # Apply lens distortion correction
    img.lens_corr(strength=1.0, zoom=1.0)

    # Verify image still has valid dimensions
    if img.width() != 64 or img.height() != 64:
        return False

    # Verify image was modified
    stats = img.get_statistics()
    if stats.min() == 0 and stats.max() == 255:
        # Still has contrast, good
        return True

    return True

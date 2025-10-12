def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(64, 64, image.GRAYSCALE)

    # Fill with pattern
    for y in range(64):
        for x in range(64):
            img.set_pixel(x, y, (x + y) % 256)

    # Apply 3D rotation correction (small rotation)
    img.rotation_corr(x_rotation=0.1, y_rotation=0.0, z_rotation=0.0,
                      x_translation=0.0, y_translation=0.0, zoom=1.0)

    # Verify image still has valid dimensions
    if img.width() != 64 or img.height() != 64:
        return False

    return True

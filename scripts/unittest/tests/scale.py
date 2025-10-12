def unittest(data_path, temp_path):
    import image

    # Create a 100x100 image
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with known pattern
    for y in range(100):
        for x in range(100):
            img.set_pixel(x, y, 128)

    # Scale down to 50x50 - scale() modifies in-place (only supports downscaling)
    img.scale(x_scale=0.5, y_scale=0.5)

    # Verify new dimensions
    if img.width() != 50 or img.height() != 50:
        return False

    # Mean should remain approximately the same
    stats = img.get_statistics()
    if stats.mean() < 120 or stats.mean() > 135:
        return False

    # Test another downscale
    img2 = image.Image(100, 100, image.GRAYSCALE)
    for y in range(100):
        for x in range(100):
            img2.set_pixel(x, y, 100)

    # Scale down to 25x25
    img2.scale(x_scale=0.25, y_scale=0.25)

    if img2.width() != 25 or img2.height() != 25:
        return False

    # Mean should be approximately preserved
    stats2 = img2.get_statistics()
    if stats2.mean() < 95 or stats2.mean() > 105:
        return False

    return True

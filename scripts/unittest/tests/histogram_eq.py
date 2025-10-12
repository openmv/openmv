def unittest(data_path, temp_path):
    import image

    # Create image with poor contrast (most pixels in narrow range)
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with values concentrated in 100-120 range (poor contrast)
    for y in range(100):
        for x in range(100):
            # Create gradient in narrow range
            img.set_pixel(x, y, 100 + (x // 5))

    # Get histogram before equalization
    stats_before = img.get_statistics()
    range_before = stats_before.max() - stats_before.min()

    # Apply histogram equalization
    img.histeq()

    # Get histogram after equalization
    stats_after = img.get_statistics()
    range_after = stats_after.max() - stats_after.min()

    # After histogram equalization:
    # Dynamic range should increase significantly (spread out the values)
    if range_after <= range_before:
        return False

    # Should use more of the 0-255 range
    if range_after < 150:  # Should be significantly wider
        return False

    return True

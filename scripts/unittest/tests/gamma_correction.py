def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with mid-range values
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 128)

    # Get original stats
    stats_orig = img.get_statistics()

    # Apply gamma correction (gamma < 1 darkens, gamma > 1 brightens)
    img.gamma_corr(gamma=0.5, contrast=1.0, brightness=0.0)

    # Get corrected stats
    stats_corrected = img.get_statistics()

    # Gamma of 0.5 should darken the image
    if stats_corrected.mean() >= stats_orig.mean():
        return False

    return True

def unittest(data_path, temp_path):
    import image

    # Create test image with poor contrast
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with low contrast pattern
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 100 + ((x + y) % 50))

    # Get original stats
    stats_orig = img.get_statistics()

    # Apply CLAHE (Contrast Limited Adaptive Histogram Equalization)
    img.histeq(adaptive=True, clip_limit=2.0)

    # Get filtered stats
    stats_filtered = img.get_statistics()

    # CLAHE should increase contrast
    orig_range = stats_orig.max() - stats_orig.min()
    filtered_range = stats_filtered.max() - stats_filtered.min()

    if filtered_range <= orig_range:
        return False

    return True

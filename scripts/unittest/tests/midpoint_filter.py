def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with gradient
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, (x + y) * 2)

    # Get original stats
    stats_orig = img.get_statistics()

    # Apply midpoint filter (average of min and max)
    img.midpoint(1, bias=1, threshold=False)

    # Verify image is modified
    stats_filtered = img.get_statistics()

    # Filtered image should have different stats
    if stats_filtered.mean() == stats_orig.mean():
        return False

    return True

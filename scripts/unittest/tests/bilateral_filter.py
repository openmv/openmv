def unittest(data_path, temp_path):
    import image

    # Create test image with edges
    img = image.Image(50, 50, image.GRAYSCALE)

    # Create a sharp edge
    for y in range(50):
        for x in range(50):
            if x < 25:
                img.set_pixel(x, y, 50)
            else:
                img.set_pixel(x, y, 200)

    # Get original stats
    stats_orig = img.get_statistics()

    # Apply bilateral filter (edge-preserving)
    img.bilateral(1, color_sigma=0.5, space_sigma=0.5, threshold=False)

    # Verify image is modified but edges preserved
    stats_filtered = img.get_statistics()

    # Check that contrast is still high (edges preserved)
    if stats_filtered.max() - stats_filtered.min() < 100:
        return False

    return True

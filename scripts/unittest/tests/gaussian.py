def unittest(data_path, temp_path):
    import image

    # Create image with sharp edges
    img = image.Image(50, 50, image.GRAYSCALE)

    # Create sharp edge pattern
    for y in range(50):
        for x in range(50):
            if x < 25:
                img.set_pixel(x, y, 0)
            else:
                img.set_pixel(x, y, 255)

    # Get stats before filtering
    stats_before = img.get_statistics()
    stdev_before = stats_before.stdev()

    # Apply Gaussian blur (kernel size 1 = 3x3, default unsharp=False)
    img.gaussian(1)

    # After Gaussian blur:
    # 1. Sharp edges should be blurred
    # 2. Standard deviation should decrease (edge is smoothed)
    stats_after = img.get_statistics()
    stdev_after = stats_after.stdev()

    # Blur should reduce standard deviation
    if stdev_after >= stdev_before:
        return False

    # Mean should remain approximately 127-128
    if stats_after.mean() < 120 or stats_after.mean() > 135:
        return False

    # Edge pixels should no longer be pure 0 or 255
    # Check some middle pixels near the edge (x=24, x=25)
    pixel_24 = img.get_pixel(24, 25)
    pixel_25 = img.get_pixel(25, 25)

    # These should be blurred (not 0 or 255)
    if pixel_24 == 0 or pixel_24 == 255 or pixel_25 == 0 or pixel_25 == 255:
        return False

    return True

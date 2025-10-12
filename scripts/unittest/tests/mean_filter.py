def unittest(data_path, temp_path):
    import image

    # Create image with noise
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with checkerboard pattern
    for y in range(50):
        for x in range(50):
            if (x + y) % 2 == 0:
                img.set_pixel(x, y, 0)
            else:
                img.set_pixel(x, y, 255)

    # Get variance before filtering
    stats_before = img.get_statistics()
    stdev_before = stats_before.stdev()

    # Apply mean filter (kernel size 1 = 3x3)
    img.mean(1)

    # Get variance after filtering
    stats_after = img.get_statistics()
    stdev_after = stats_after.stdev()

    # Mean filter should reduce variance/stdev
    if stdev_after >= stdev_before:
        return False

    # Mean should remain approximately the same (around 127-128)
    if stats_after.mean() < 120 or stats_after.mean() > 135:
        return False

    return True

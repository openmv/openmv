def unittest(data_path, temp_path):
    import image

    # Create image with salt-and-pepper noise
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with base value 128
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 128)

    # Add salt-and-pepper noise (random black and white pixels)
    noise_pixels = [
        (5, 5),
        (10, 10),
        (15, 15),
        (20, 20),
        (25, 25),
        (5, 10),
        (10, 15),
        (15, 20),
        (20, 25),
        (25, 30),
    ]

    for i, (x, y) in enumerate(noise_pixels):
        if i % 2 == 0:
            img.set_pixel(x, y, 0)  # Black noise
        else:
            img.set_pixel(x, y, 255)  # White noise

    # Get variance before filtering
    stats_before = img.get_statistics()
    stdev_before = stats_before.stdev()

    # Apply median filter (kernel size 1 = 3x3)
    img.median(1)

    # Get variance after filtering
    stats_after = img.get_statistics()
    stdev_after = stats_after.stdev()

    # Median filter should reduce variance (removes noise)
    if stdev_after >= stdev_before:
        return False

    # Mean should remain close to 128
    if stats_after.mean() < 125 or stats_after.mean() > 131:
        return False

    return True

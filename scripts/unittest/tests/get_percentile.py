def unittest(data_path, temp_path):
    import image

    # Create test image with known distribution
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with values 0-99 repeated
    for y in range(100):
        for x in range(100):
            img.set_pixel(x, y, (x + y) % 100)

    # Get histogram
    hist = img.get_histogram()

    # Get percentile (50th percentile should be around median)
    percentile = hist.get_percentile(0.5)

    # Verify percentile object
    if not hasattr(percentile, 'value'):
        return False

    # 50th percentile should be around 50
    if abs(percentile.value() - 50) > 20:
        return False

    return True

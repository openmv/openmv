def unittest(data_path, temp_path):
    import image

    # Create test image with bimodal distribution
    img = image.Image(100, 100, image.GRAYSCALE)

    # Create two distinct regions (bimodal)
    for y in range(100):
        for x in range(100):
            if x < 50:
                img.set_pixel(x, y, 50)
            else:
                img.set_pixel(x, y, 200)

    # Get histogram
    hist = img.get_histogram()

    # Get automatic threshold (Otsu's method)
    threshold = hist.get_threshold()

    # Verify threshold object
    if not hasattr(threshold, 'value'):
        return False

    # Threshold should be between the two peaks (50 and 200)
    if threshold.value() < 50 or threshold.value() > 200:
        return False

    return True

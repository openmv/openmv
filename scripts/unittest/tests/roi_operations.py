def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with gradient
    for y in range(100):
        for x in range(100):
            img.set_pixel(x, y, (x + y) % 256)

    # Get statistics for ROI (region of interest)
    roi = (25, 25, 50, 50)  # x, y, w, h
    stats = img.get_statistics(roi=roi)

    # Verify stats are calculated
    if stats.mean() < 0 or stats.mean() > 255:
        return False

    # Test histogram with ROI
    hist = img.get_histogram(roi=roi)

    if hist is None:
        return False

    # Test copy with ROI
    img_roi = img.copy(roi=roi)

    # ROI copy should have dimensions 50x50
    if img_roi.width() != 50 or img_roi.height() != 50:
        return False

    return True

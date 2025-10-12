def unittest(data_path, temp_path):
    import image

    # Create test image with a line of white pixels
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with black
    for y in range(100):
        for x in range(100):
            img.set_pixel(x, y, 0)

    # Draw a line at roughly 30 degrees using draw_line
    # Draw from (10, 10) to (90, 56) - this is approximately 30 degrees
    img.draw_line(10, 10, 90, 56, color=255, thickness=5)

    # Get linear regression on white pixels
    line = img.get_regression([(200, 255)], invert=False,
                               area_threshold=10, pixels_threshold=10)

    # Verify regression found the line
    if line is None:
        return False

    # Line at 30 degrees from horizontal should have theta around 120 degrees
    theta = line.theta()

    # Allow generous tolerance since exact angle detection varies
    if abs(theta - 120) > 30:
        return False

    return True

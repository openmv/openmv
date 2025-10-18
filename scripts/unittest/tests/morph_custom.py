def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with pattern
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 128)

    # Define custom kernel (3x3 box blur)
    kernel = [1, 1, 1,
              1, 1, 1,
              1, 1, 1]

    # Apply custom morphological operation
    img.morph(1, kernel)

    # Verify image is processed
    stats = img.get_statistics()

    # Should still be around 128 after blur
    if abs(stats.mean() - 128) > 10:
        return False

    return True

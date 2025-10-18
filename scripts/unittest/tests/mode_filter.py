def unittest(data_path, temp_path):
    import image

    # Create test image with specific pattern
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with pattern (repeating values to have a clear mode)
    for y in range(50):
        for x in range(50):
            # Create regions with dominant values
            if x < 25:
                img.set_pixel(x, y, 100)
            else:
                img.set_pixel(x, y, 200)

    # Apply mode filter (most common value in kernel)
    img.mode(1, threshold=False)

    # Verify image is modified
    stats = img.get_statistics()
    if stats.mean() < 50 or stats.mean() > 250:
        return False

    return True

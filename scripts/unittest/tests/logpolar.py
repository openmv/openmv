def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(64, 64, image.GRAYSCALE)

    # Fill with radial pattern
    for y in range(64):
        for x in range(64):
            dx = x - 32
            dy = y - 32
            dist = int((dx*dx + dy*dy) ** 0.5)
            img.set_pixel(x, y, (dist * 8) % 256)

    # Apply log-polar transform
    img.logpolar(reverse=False)

    # Verify image still valid
    if img.width() != 64 or img.height() != 64:
        return False

    return True

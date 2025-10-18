def unittest(data_path, temp_path):
    import image

    # Create RGB565 image as baseline
    img = image.Image(64, 64, image.RGB565)

    # Fill with pattern
    for y in range(64):
        for x in range(64):
            r = (x * 8) & 0xF8
            g = (y * 4) & 0xFC
            b = ((x + y) * 8) & 0xF8
            color = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | (b >> 3)
            img.set_pixel(x, y, color)

    # Verify RGB565 format
    if img.format() != image.RGB565:
        return False

    # Convert to grayscale (similar to debayering end result)
    img_gray = img.to_grayscale()

    if img_gray.format() != image.GRAYSCALE:
        return False

    return True

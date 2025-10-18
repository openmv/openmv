def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(48, 48, image.RGB565, copy_to_fb=True)

    # Fill with pattern
    for y in range(48):
        for x in range(48):
            color = ((x * 8) << 8) | ((y * 4) << 3) | ((x + y) >> 3)
            img.set_pixel(x, y, color)

    # Compress to PNG
    img = img.to_png()

    # Verify it's compressed
    if img.format() != image.PNG:
        return False

    # Decompress back
    img.to_rgb565()

    # Verify dimensions (PNG is lossless)
    if img.width() != 48 or img.height() != 48:
        return False

    if img.format() != image.RGB565:
        return False

    # Verify pixel values match (PNG is lossless, but RGB565 has precision loss)
    match_count = 0
    for y in range(48):
        for x in range(48):
            original_color = ((x * 8) << 8) | ((y * 4) << 3) | ((x + y) >> 3)
            r, g, b = img.get_pixel(x, y)

            # Extract original RGB565 components
            orig_r = (original_color >> 8) & 0xF8
            orig_g = (original_color >> 3) & 0xFC
            orig_b = (original_color << 3) & 0xF8

            # Compare individual color components (allow small difference per channel)
            if abs(r - orig_r) < 8 and abs(g - orig_g) < 8 and abs(b - orig_b) < 8:
                match_count += 1

    # Most pixels should match closely (lower threshold due to RGB565 precision)
    if match_count < (48 * 48 * 0.9):
        return False

    return True

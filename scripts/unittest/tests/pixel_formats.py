def unittest(data_path, temp_path):
    import image

    # Create RGB565 image
    img_rgb = image.Image(40, 40, image.RGB565, copy_to_fb=True)

    # Fill with pattern
    for y in range(40):
        for x in range(40):
            r = (x * 8) & 0xF8
            g = (y * 4) & 0xFC
            b = ((x + y) * 8) & 0xF8
            color = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | (b >> 3)
            img_rgb.set_pixel(x, y, color)

    # Convert to grayscale
    img_gray = img_rgb.to_grayscale()
    if img_gray.format() != image.GRAYSCALE:
        return False
    if img_gray.width() != 40 or img_gray.height() != 40:
        return False

    # Convert to binary
    img_bin = img_gray.to_bitmap()
    if img_bin.format() != image.BINARY:
        return False
    if img_bin.width() != 40 or img_bin.height() != 40:
        return False

    # Convert binary back to grayscale
    img_gray2 = img_bin.to_grayscale()
    if img_gray2.format() != image.GRAYSCALE:
        return False

    # Convert grayscale back to RGB565
    img_rgb2 = img_gray2.to_rgb565()
    if img_rgb2.format() != image.RGB565:
        return False

    return True

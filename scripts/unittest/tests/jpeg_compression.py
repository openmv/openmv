def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(64, 64, image.RGB565)

    # Fill with pattern
    for y in range(64):
        for x in range(64):
            color = ((x * 8) << 8) | ((y * 4) << 3) | ((x + y) >> 3)
            img.set_pixel(x, y, color)

    # Compress to JPEG with quality 50
    img = img.to_jpeg(quality=50)

    # Verify it's compressed
    if img.format() != image.JPEG:
        return False

    # JPEG size should be smaller than uncompressed
    jpeg_size = img.size()
    uncompressed_size = 64 * 64 * 2  # RGB565 = 2 bytes per pixel

    if jpeg_size >= uncompressed_size:
        return False

    # Decompress
    img = image.Image(data_path + "/compressed.jpeg", copy_to_fb=True)
    img.to_rgb565()

    # Verify dimensions
    if img.width() != 320 or img.height() != 270:
        return False

    if img.format() != image.RGB565:
        return False

    return True

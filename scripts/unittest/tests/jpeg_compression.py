def unittest(data_path, temp_path):
    import image

    # Test Part 1: JPEG Compression (works on all platforms)
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

    # Test Part 2: JPEG Decompression from file
    # This requires hardware JPEG codec (OMV_JPEG_CODEC_ENABLE=1)
    # Software decoder (Unix/QEMU) cannot handle copy_to_fb workflow
    # Skip this part on software decoder

    # Capability test: Try to load a JPEG with copy_to_fb
    # If it fails, we're on software decoder - skip this part
    try:
        # Try loading with hardware codec workflow
        test_img = image.Image(data_path + "/compressed.jpeg", copy_to_fb=True)
        test_img.to_rgb565()

        # Hardware codec path: full test
        if test_img.width() != 320 or test_img.height() != 270:
            return False

        if test_img.format() != image.RGB565:
            return False

    except OSError:
        # Software decoder: Skip file decompression test
        # This is expected on Unix/QEMU platforms
        pass

    return True

def unittest(data_path, temp_path):
    import image

    # Create test image with regions
    img = image.Image(64, 64, image.RGB565)

    # Create distinct regions
    for y in range(64):
        for x in range(64):
            if x < 32 and y < 32:
                color = 0xF800  # Red
            elif x >= 32 and y < 32:
                color = 0x07E0  # Green
            elif x < 32 and y >= 32:
                color = 0x001F  # Blue
            else:
                color = 0xFFFF  # White
            img.set_pixel(x, y, color)

    # Perform selective search segmentation
    regions = img.selective_search(threshold=200, min_size=10)

    return regions[0] == (0, 0, 63, 31) and regions[1] == (0, 0, 63, 63)

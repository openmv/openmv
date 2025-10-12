def unittest(data_path, temp_path):
    import image

    # Create test image with region to fill
    img = image.Image(50, 50, image.GRAYSCALE)

    # Create a bounded region
    for y in range(50):
        for x in range(50):
            if 10 <= x <= 40 and 10 <= y <= 40:
                img.set_pixel(x, y, 100)
            else:
                img.set_pixel(x, y, 200)

    # Flood fill from center
    img.flood_fill(25, 25, seed_threshold=0.05, floating_threshold=0.05,
                   color=255, invert=False, clear_background=False)

    # Verify that central region was filled
    center_pixel = img.get_pixel(25, 25)
    if center_pixel != 255:
        return False

    # Verify border was not filled
    border_pixel = img.get_pixel(5, 5)
    if border_pixel == 255:
        return False

    return True

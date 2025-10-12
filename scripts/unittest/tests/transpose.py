def unittest(data_path, temp_path):
    import image

    # Create non-square test image
    img = image.Image(60, 40, image.GRAYSCALE)

    # Fill with pattern
    for y in range(40):
        for x in range(60):
            img.set_pixel(x, y, (x + y) % 256)

    # Transpose image (swap width and height)
    img_t = img.copy(x_scale=1.0, y_scale=1.0, hint=image.TRANSPOSE)

    # Verify dimensions are swapped
    if img_t.width() != 40 or img_t.height() != 60:
        return False

    return True

def unittest(data_path, temp_path):
    import image

    # Create grayscale test image
    img = image.Image(50, 50, image.RGB565)

    # Fill with gradient
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, (x + y) * 2)

    # Test rainbow palette
    img_rainbow = img.to_rainbow()
    if img_rainbow.format() != image.RGB565:
        return False
    if img_rainbow.width() != 50 or img_rainbow.height() != 50:
        return False

    # Test ironbow palette
    img_ironbow = img.to_ironbow()
    if img_ironbow.format() != image.RGB565:
        return False

    return True

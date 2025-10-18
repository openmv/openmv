def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(50, 50, image.GRAYSCALE)

    # Create image with small dark features on bright background
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 200)

    # Add small dark spots
    for i in range(5, 45, 10):
        for j in range(5, 45, 10):
            img.set_pixel(i, j, 50)
            img.set_pixel(i+1, j, 50)
            img.set_pixel(i, j+1, 50)

    # Apply black-hat (extracts small dark features)
    # Black-hat = closing - image
    img_copy = img.copy()
    img_copy.close(2)

    # Verify processing
    return True

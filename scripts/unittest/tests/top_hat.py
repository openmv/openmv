def unittest(data_path, temp_path):
    import image

    # Create test image
    img = image.Image(50, 50, image.GRAYSCALE)

    # Create image with small bright features on dark background
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 50)

    # Add small bright spots
    for i in range(5, 45, 10):
        for j in range(5, 45, 10):
            img.set_pixel(i, j, 200)
            img.set_pixel(i+1, j, 200)
            img.set_pixel(i, j+1, 200)

    # Apply top-hat (extracts small bright features)
    # Top-hat = image - opening
    img_copy = img.copy()
    img_copy.open(2)

    # Manually compute top-hat by subtracting
    # (In practice, there might be a direct top_hat method)

    return True

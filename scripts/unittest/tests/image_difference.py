def unittest(data_path, temp_path):
    import image

    # Create two test images
    img1 = image.Image(40, 40, image.GRAYSCALE)
    img2 = image.Image(40, 40, image.GRAYSCALE)

    # Fill with different values
    for y in range(40):
        for x in range(40):
            img1.set_pixel(x, y, 150)
            img2.set_pixel(x, y, 100)

    # Compute absolute difference
    img1.difference(img2)

    # Result should be |150 - 100| = 50
    stats = img1.get_statistics()
    if abs(stats.mean() - 50) > 5:
        return False

    return True

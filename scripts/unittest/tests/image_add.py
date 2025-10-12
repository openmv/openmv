def unittest(data_path, temp_path):
    import image

    # Create two grayscale images with known values
    img1 = image.Image(50, 50, image.GRAYSCALE)
    img2 = image.Image(50, 50, image.GRAYSCALE)

    # Fill img1 with 50
    for y in range(50):
        for x in range(50):
            img1.set_pixel(x, y, 50)

    # Fill img2 with 100
    for y in range(50):
        for x in range(50):
            img2.set_pixel(x, y, 100)

    # Add img2 to img1
    img1.add(img2)

    # Verify result is 150
    stats = img1.get_statistics()
    if stats.mean() != 150 or stats.min() != 150 or stats.max() != 150:
        return False

    # Test saturation at 255
    img3 = image.Image(50, 50, image.GRAYSCALE)
    img4 = image.Image(50, 50, image.GRAYSCALE)

    for y in range(50):
        for x in range(50):
            img3.set_pixel(x, y, 200)
            img4.set_pixel(x, y, 100)

    img3.add(img4)

    # Should saturate at 255
    stats2 = img3.get_statistics()
    if stats2.max() != 255:
        return False

    return True

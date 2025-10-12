def unittest(data_path, temp_path):
    import image

    # Create two test images
    img1 = image.Image(40, 40, image.GRAYSCALE)
    img2 = image.Image(40, 40, image.GRAYSCALE)

    # Fill with different patterns
    for y in range(40):
        for x in range(40):
            img1.set_pixel(x, y, 100)
            img2.set_pixel(x, y, 150)

    # Test min operation
    img_min = img1.copy()
    img_min.min(img2)

    # Result should be minimum of two images (100)
    stats_min = img_min.get_statistics()
    if stats_min.mean() != 100:
        return False

    # Test max operation
    img_max = img1.copy()
    img_max.max(img2)

    # Result should be maximum of two images (150)
    stats_max = img_max.get_statistics()
    if stats_max.mean() != 150:
        return False

    return True

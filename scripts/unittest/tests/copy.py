def unittest(data_path, temp_path):
    import image

    # Create original image
    img = image.Image(50, 50, image.GRAYSCALE)

    # Fill with value 100
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 100)

    # Create a copy
    img_copy = img.copy()

    # Verify copy has same dimensions and values
    if img_copy.width() != 50 or img_copy.height() != 50:
        return False

    stats = img_copy.get_statistics()
    if stats.mean() != 100:
        return False

    # Modify original image
    for y in range(50):
        for x in range(50):
            img.set_pixel(x, y, 200)

    # Verify copy is independent (still has value 100)
    stats_copy = img_copy.get_statistics()
    if stats_copy.mean() != 100:
        return False

    # Verify original changed to 200
    stats_orig = img.get_statistics()
    if stats_orig.mean() != 200:
        return False

    return True

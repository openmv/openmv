def unittest(data_path, temp_path):
    import image

    # Create two grayscale images
    img1 = image.Image(50, 50, image.GRAYSCALE)
    img2 = image.Image(50, 50, image.GRAYSCALE)

    # Fill img1 with 150
    for y in range(50):
        for x in range(50):
            img1.set_pixel(x, y, 150)

    # Fill img2 with 50
    for y in range(50):
        for x in range(50):
            img2.set_pixel(x, y, 50)

    # Subtract img2 from img1
    img1.sub(img2)

    # Verify result is 100
    stats = img1.get_statistics()
    if stats.mean() != 100 or stats.min() != 100 or stats.max() != 100:
        return False

    # Test underflow protection at 0
    img3 = image.Image(50, 50, image.GRAYSCALE)
    img4 = image.Image(50, 50, image.GRAYSCALE)

    for y in range(50):
        for x in range(50):
            img3.set_pixel(x, y, 50)
            img4.set_pixel(x, y, 100)

    img3.sub(img4)

    # Should clamp at 0
    stats2 = img3.get_statistics()
    if stats2.min() != 0:
        return False

    return True

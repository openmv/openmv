def unittest(data_path, temp_path):
    import image

    # Create a test image with distinct regions
    img = image.Image(100, 100, image.GRAYSCALE)

    # Fill with pattern: different values in different quadrants
    for y in range(100):
        for x in range(100):
            if x < 50 and y < 50:
                img.set_pixel(x, y, 50)  # Top-left
            elif x >= 50 and y < 50:
                img.set_pixel(x, y, 100)  # Top-right
            elif x < 50 and y >= 50:
                img.set_pixel(x, y, 150)  # Bottom-left
            else:
                img.set_pixel(x, y, 200)  # Bottom-right

    # Make a copy and crop top-left quadrant (50x50 region starting at 0,0)
    img1 = img.copy()
    img1.crop(roi=(0, 0, 50, 50))

    # Verify dimensions
    if img1.width() != 50 or img1.height() != 50:
        return False

    # Verify all pixels are 50 (top-left quadrant value)
    stats = img1.get_statistics()
    if stats.mean() != 50 or stats.min() != 50 or stats.max() != 50:
        return False

    # Crop bottom-right quadrant from original (need fresh copy)
    img2 = img.copy()
    img2.crop(roi=(50, 50, 50, 50))

    if img2.width() != 50 or img2.height() != 50:
        return False

    stats2 = img2.get_statistics()
    if stats2.mean() != 200:
        return False

    return True

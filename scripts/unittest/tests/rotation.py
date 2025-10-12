def unittest(data_path, temp_path):
    import image

    # Create a simple asymmetric pattern to test rotation
    img = image.Image(60, 60, image.GRAYSCALE)

    # Fill with a pattern: top-left corner bright, rest dark
    for y in range(60):
        for x in range(60):
            if x < 20 and y < 20:
                img.set_pixel(x, y, 200)  # Bright top-left
            else:
                img.set_pixel(x, y, 50)  # Dark everywhere else

    # Test 90 degree rotation
    img_90 = img.copy()
    img_90.rotation_corr(z_rotation=90)

    # After 90° clockwise rotation, top-left becomes top-right
    # Check that the bright region moved
    # Top-right should now be bright
    pixel_tr = img_90.get_pixel(55, 5)
    if pixel_tr < 150:  # Should be bright
        return False

    # Test 180 degree rotation
    img_180 = img.copy()
    img_180.rotation_corr(z_rotation=180)

    # After 180° rotation, top-left becomes bottom-right
    pixel_br = img_180.get_pixel(55, 55)
    if pixel_br < 150:  # Should be bright
        return False

    # Original top-right should now be dark after 180° rotation
    pixel_tl_after = img_180.get_pixel(5, 5)
    if pixel_tl_after > 100:  # Should be dark
        return False

    return True

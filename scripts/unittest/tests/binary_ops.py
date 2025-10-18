def unittest(data_path, temp_path):
    import image

    # Test AND operation
    img1 = image.Image(50, 50, image.GRAYSCALE)
    img2 = image.Image(50, 50, image.GRAYSCALE)

    # Fill with 0b11110000 (240) and 0b00111100 (60)
    for y in range(50):
        for x in range(50):
            img1.set_pixel(x, y, 240)
            img2.set_pixel(x, y, 60)

    img1.b_and(img2)
    # 240 & 60 = 0b11110000 & 0b00111100 = 0b00110000 = 48
    stats = img1.get_statistics()
    if stats.mean() != 48:
        return False

    # Test OR operation
    img3 = image.Image(50, 50, image.GRAYSCALE)
    img4 = image.Image(50, 50, image.GRAYSCALE)

    for y in range(50):
        for x in range(50):
            img3.set_pixel(x, y, 240)  # 0b11110000
            img4.set_pixel(x, y, 15)  # 0b00001111

    img3.b_or(img4)
    # 240 | 15 = 0b11110000 | 0b00001111 = 0b11111111 = 255
    stats2 = img3.get_statistics()
    if stats2.mean() != 255:
        return False

    # Test XOR operation
    img5 = image.Image(50, 50, image.GRAYSCALE)
    img6 = image.Image(50, 50, image.GRAYSCALE)

    for y in range(50):
        for x in range(50):
            img5.set_pixel(x, y, 255)  # 0b11111111
            img6.set_pixel(x, y, 170)  # 0b10101010

    img5.b_xor(img6)
    # 255 ^ 170 = 0b11111111 ^ 0b10101010 = 0b01010101 = 85
    stats3 = img5.get_statistics()
    if stats3.mean() != 85:
        return False

    return True

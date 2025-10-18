def unittest(data_path, temp_path):
    import image

    # Test 1: File-based write and read
    test_file = temp_path + "/test_stream.bin"

    # Create test images with known patterns (not in framebuffer)
    img1 = image.Image(40, 40, image.RGB565)
    for y in range(40):
        for x in range(40):
            r = (x * 8) & 0xF8
            g = (y * 4) & 0xFC
            b = ((x + y) * 8) & 0xF8
            color = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | (b >> 3)
            img1.set_pixel(x, y, color)

    img2 = image.Image(40, 40, image.RGB565)
    for y in range(40):
        for x in range(40):
            r = ((40 - x) * 8) & 0xF8
            g = ((40 - y) * 4) & 0xFC
            b = ((x ^ y) * 8) & 0xF8
            color = ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | (b >> 3)
            img2.set_pixel(x, y, color)

    img3 = image.Image(40, 40, image.RGB565)
    for y in range(40):
        for x in range(40):
            val = ((x + y) % 2) * 31
            color = ((val << 11) & 0xF800) | ((val << 5) & 0x07E0) | val
            img3.set_pixel(x, y, color)

    # Write images to file
    stream = image.ImageIO(test_file, "w")
    stream.write(img1)
    stream.write(img2)
    stream.write(img3)
    stream.close()

    # Read images back from file
    stream = image.ImageIO(test_file, "r")
    read_img1 = stream.read(copy_to_fb=False)
    read_img2 = stream.read(copy_to_fb=False)
    read_img3 = stream.read(copy_to_fb=False)
    stream.close()

    # Verify images match
    if read_img1 is None or read_img2 is None or read_img3 is None:
        return False

    stats1 = img1.difference(read_img1).get_statistics()
    if stats1.max() != 0 or stats1.min() != 0:
        return False

    stats2 = img2.difference(read_img2).get_statistics()
    if stats2.max() != 0 or stats2.min() != 0:
        return False

    stats3 = img3.difference(read_img3).get_statistics()
    if stats3.max() != 0 or stats3.min() != 0:
        return False

    # Test 2: Memory stream with seek
    mem_stream = image.ImageIO((40, 40, image.RGB565), 3)

    # Write images to memory
    mem_stream.write(img1)
    mem_stream.write(img2)
    mem_stream.write(img3)

    # Seek to beginning and read back
    mem_stream.seek(0)
    mem_img1 = mem_stream.read(copy_to_fb=False)
    mem_img2 = mem_stream.read(copy_to_fb=False)
    mem_img3 = mem_stream.read(copy_to_fb=False)

    # Verify images match
    if mem_img1 is None or mem_img2 is None or mem_img3 is None:
        return False

    stats1 = img1.difference(mem_img1).get_statistics()
    if stats1.max() != 0 or stats1.min() != 0:
        return False

    stats2 = img2.difference(mem_img2).get_statistics()
    if stats2.max() != 0 or stats2.min() != 0:
        return False

    stats3 = img3.difference(mem_img3).get_statistics()
    if stats3.max() != 0 or stats3.min() != 0:
        return False

    # Test 3: Seek to specific frame
    mem_stream.seek(0)
    mem_stream.read(copy_to_fb=False)  # Skip first
    mem_img2_direct = mem_stream.read(copy_to_fb=False)

    stats2 = img2.difference(mem_img2_direct).get_statistics()
    if stats2.max() != 0 or stats2.min() != 0:
        return False

    # Test 4: Grayscale images
    gray_file = temp_path + "/test_gray.bin"

    gray1 = image.Image(30, 30, image.GRAYSCALE)
    for y in range(30):
        for x in range(30):
            gray1.set_pixel(x, y, (x * y) % 256)

    gray2 = image.Image(30, 30, image.GRAYSCALE)
    for y in range(30):
        for x in range(30):
            gray2.set_pixel(x, y, (x + y) % 256)

    # Write and read grayscale
    gray_stream = image.ImageIO(gray_file, "w")
    gray_stream.write(gray1)
    gray_stream.write(gray2)
    gray_stream.close()

    gray_stream = image.ImageIO(gray_file, "r")
    read_gray1 = gray_stream.read(copy_to_fb=False)
    read_gray2 = gray_stream.read(copy_to_fb=False)
    gray_stream.close()

    if read_gray1 is None or read_gray2 is None:
        return False

    stats_g1 = gray1.difference(read_gray1).get_statistics()
    if stats_g1.max() != 0 or stats_g1.min() != 0:
        return False

    stats_g2 = gray2.difference(read_gray2).get_statistics()
    if stats_g2.max() != 0 or stats_g2.min() != 0:
        return False

    return True

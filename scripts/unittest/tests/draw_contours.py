def unittest(data_path, temp_path):
    import image

    img = image.Image(9, 9, image.GRAYSCALE)

    for y in range(1, 8):
        for x in range(1, 8):
            img.set_pixel((x, y), 255)
    img.set_pixel((4, 4), 0)

    img.draw_contours([(200, 255)], color=128, roi=(1, 1, 7, 7))

    outline = 0
    inside = 0
    background = 0

    for y in range(9):
        for x in range(9):
            pixel = img.get_pixel((x, y))
            if pixel == 128:
                outline += 1
            elif pixel == 255:
                inside += 1
            elif pixel == 0:
                background += 1
            else:
                return False

    return (
        outline == 32
        and inside == 16
        and background == 33
        and img.get_pixel((2, 2)) == 255
        and img.get_pixel((3, 3)) == 128
        and img.get_pixel((4, 4)) == 0
    )

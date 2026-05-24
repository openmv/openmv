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

    direct_ok = (
        outline == 32
        and inside == 16
        and background == 33
        and img.get_pixel((2, 2)) == 255
        and img.get_pixel((3, 3)) == 128
        and img.get_pixel((4, 4)) == 0
    )

    src = image.Image(9, 9, image.GRAYSCALE)
    for y in range(1, 8):
        for x in range(1, 8):
            src.set_pixel((x, y), 255)
    src.set_pixel((4, 4), 0)
    src.set_pixel((0, 0), 255)

    overlay = image.Image(9, 9, image.RGB565)
    mask = image.Image(9, 9, image.BINARY)
    overlay.draw_contours([(200, 255)], color=(255, 0, 0), image=src, mask=mask, seed=(2, 2))

    display = image.Image(9, 9, image.RGB565)
    display.draw_image(src, 0, 0)
    display.draw_image(overlay, 0, 0, mask=mask)

    overlay_count = 0
    mask_count = 0
    for y in range(9):
        for x in range(9):
            if overlay.get_pixel((x, y)) == (255, 0, 0):
                overlay_count += 1
            if mask.get_pixel((x, y)):
                mask_count += 1

    overlay_ok = (
        overlay_count == 32
        and mask_count == 32
        and src.get_pixel((3, 3)) == 255
        and mask.get_pixel((0, 0)) == 0
        and overlay.get_pixel((0, 0)) == (0, 0, 0)
        and overlay.get_pixel((2, 2)) == (0, 0, 0)
        and display.get_pixel((2, 2)) == (255, 255, 255)
        and display.get_pixel((3, 3)) == (255, 0, 0)
    )

    return direct_ok and overlay_ok

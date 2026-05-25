def unittest(data_path, temp_path):
    import image

    def make_ring():
        img = image.Image(9, 9, image.GRAYSCALE)
        for y in range(1, 8):
            for x in range(1, 8):
                img.set_pixel((x, y), 255)
        img.set_pixel((4, 4), 0)
        return img

    img = make_ring()
    blobs = img.find_blobs(
        [(200, 255)],
        x_stride=1,
        y_stride=1,
        pixels_threshold=1,
        area_threshold=1,
        contours=True,
    )
    if len(blobs) != 1 or blobs[0].contour is None:
        return False

    img.draw_contours(blobs[0], color=128)

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
        outline == 28
        and inside == 20
        and background == 33
        and img.get_pixel((2, 2)) == 255
        and img.get_pixel((4, 3)) == 128
        and img.get_pixel((3, 3)) == 255
        and img.get_pixel((4, 4)) == 0
    )

    src = make_ring()
    overlay_blob = src.find_blobs(
        [(200, 255)],
        x_stride=1,
        y_stride=1,
        pixels_threshold=1,
        area_threshold=1,
        contours=True,
    )[0]

    overlay = image.Image(9, 9, image.RGB565)
    mask = image.Image(9, 9, image.BINARY)
    overlay.draw_contours(overlay_blob, color=(255, 0, 0), mask=mask)

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
        overlay_count == 28
        and mask_count == 28
        and src.get_pixel((3, 3)) == 255
        and overlay.get_pixel((2, 2)) == (0, 0, 0)
        and display.get_pixel((2, 2)) == (255, 255, 255)
        and display.get_pixel((4, 3)) == (255, 0, 0)
    )

    no_contour = make_ring()
    no_contour_blob = no_contour.find_blobs(
        [(200, 255)],
        x_stride=1,
        y_stride=1,
        pixels_threshold=1,
        area_threshold=1,
    )[0]
    no_contour.draw_contours(no_contour_blob, color=128)
    no_contour_ok = no_contour_blob.contour is None and no_contour.get_pixel((1, 1)) == 255

    merged_src = image.Image(7, 4, image.GRAYSCALE)
    for y in range(1, 3):
        for x in (1, 2, 4, 5):
            merged_src.set_pixel((x, y), 255)
    merged = merged_src.find_blobs(
        [(200, 255)],
        x_stride=1,
        y_stride=1,
        pixels_threshold=1,
        area_threshold=1,
        merge=True,
        margin=2,
        contours=True,
    )
    if len(merged) != 1 or merged[0].contour is None:
        return False

    merged_draw = image.Image(7, 4, image.GRAYSCALE)
    merged_draw.draw_contours(merged[0], color=128)
    merged_count = 0
    for y in range(4):
        for x in range(7):
            if merged_draw.get_pixel((x, y)) == 128:
                merged_count += 1

    return direct_ok and overlay_ok and no_contour_ok and merged_count == 8

def unittest(data_path, temp_path):
    import image

    # Check each pixel, including the ROI boundary and a hole in the blob.
    for pixformat, foreground, threshold in (
        (image.BINARY, 1, (1, 1)),
        (image.GRAYSCALE, 255, (255, 255)),
        (image.RGB565, (255, 255, 255), (100, 100, -128, 127, -128, 127)),
    ):
        src = image.Image(11, 11, pixformat)
        for y in range(1, 10):
            for x in range(1, 10):
                src.set_pixel((x, y), foreground)
        src.set_pixel((5, 5), 0)
        blobs = src.find_blobs(
            [threshold], roi=(2, 2, 7, 7), x_stride=1, y_stride=1,
            pixels_threshold=1, area_threshold=1, contours=True,
        )
        if len(blobs) != 1:
            return False
        dst = image.Image(11, 11, image.GRAYSCALE)
        dst.draw_contours(blobs[0], color=255)
        for y in range(11):
            for x in range(11):
                outer = 2 <= x <= 8 and 2 <= y <= 8 and (x in (2, 8) or y in (2, 8))
                hole = (x, y) in ((5, 4), (4, 5), (6, 5), (5, 6))
                if bool(dst.get_pixel((x, y))) != (outer or hole):
                    return False

    # A one-pixel blob has an empty delta stream but still draws its start.
    src = image.Image(3, 3, image.GRAYSCALE)
    src.set_pixel((1, 1), 255)
    blob = src.find_blobs(
        [(255, 255)], x_stride=1, y_stride=1,
        pixels_threshold=1, area_threshold=1, contours=True,
    )[0]
    dst = image.Image(3, 3, image.GRAYSCALE)
    dst.draw_contours(blob, color=255)
    return dst.get_pixel((1, 1)) == 255 and dst.get_pixel((0, 0)) == 0

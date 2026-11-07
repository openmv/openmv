def unittest(data_path, temp_path):
    import image

    # Two threshold components touch along a vertical edge. Merging retains
    # each component's contour rather than tracing the outline of their union.
    src = image.Image(10, 6, image.GRAYSCALE)
    src.draw_rectangle((1, 1, 4, 4), color=64, fill=True)
    src.draw_rectangle((5, 1, 4, 4), color=192, fill=True)
    blobs = src.find_blobs(
        [(64, 64), (192, 192)], x_stride=1, y_stride=1,
        pixels_threshold=1, area_threshold=1, merge=True, margin=1, contours=True,
    )
    if len(blobs) != 1 or blobs[0].count != 2:
        return False

    dst = image.Image(10, 6, image.GRAYSCALE)
    dst.draw_contours(blobs[0], color=255)
    for y in range(6):
        for x in range(10):
            left = 1 <= x <= 4 and 1 <= y <= 4 and (x in (1, 4) or y in (1, 4))
            right = 5 <= x <= 8 and 1 <= y <= 4 and (x in (5, 8) or y in (1, 4))
            if bool(dst.get_pixel((x, y))) != (left or right):
                return False
    return True

def unittest(data_path, temp_path):
    import image
    import time

    img = image.Image(data_path + "/apriltags.pgm", copy_to_fb=True)
    tags = img.find_apriltags()

    if len(tags) != 6:
        return False

    expected = [
        (0, 8, 494, 308, 34, 39, 510, 327),
        (0, 8, 567, 333, 37, 40, 585, 352),
        (0, 8, 302, 324, 36, 38, 320, 342),
        (0, 8, 229, 319, 36, 38, 246, 338),
        (0, 8, 469, 306, 18, 42, 478, 327),
        (0, 8, 324, 194, 37, 31, 342, 209),
    ]

    for t, e in zip(tags, expected):
        if (t.id, t.family, t.x, t.y, t.w, t.h, t.cx, t.cy) != e:
            return False

    return True

def unittest(data_path, temp_path):
    import image

    def grid(z_rotation):
        # A 3x3 grid of 3 px black lines on white. The outer frame and all nine
        # cells are rectangles, so find_rects() must return ten of them.
        img = image.Image(160, 120, image.GRAYSCALE)
        img.draw_rectangle((0, 0, 160, 120), color=255, fill=True)
        for i in range(4):
            x = 45 + (i * 23)
            img.draw_line((x, 25, x, 94), color=0, thickness=3)
            img.draw_line((45, x - 20, 114, x - 20), color=0, thickness=3)
        if z_rotation:
            img.rotation_corr(z_rotation=z_rotation)
        return img

    expected = {
        0: [
            (44, 24, 72, 72, 291718),
            (47, 27, 20, 20, 83326),
            (47, 50, 20, 20, 83326),
            (47, 73, 20, 20, 83326),
            (70, 27, 20, 20, 83326),
            (70, 50, 20, 20, 83326),
            (70, 73, 20, 20, 83326),
            (93, 27, 20, 20, 83326),
            (93, 50, 20, 20, 83326),
            (93, 73, 20, 20, 83326),
        ],
        15: [
            (36, 16, 88, 88, 237745),
            (40, 64, 24, 24, 76433),
            (46, 42, 24, 24, 70012),
            (52, 19, 24, 25, 69986),
            (62, 70, 25, 24, 73516),
            (68, 48, 24, 24, 75124),
            (74, 25, 24, 25, 73676),
            (84, 76, 25, 24, 71076),
            (90, 54, 25, 24, 71455),
            (96, 32, 25, 24, 71511),
        ],
    }

    for z_rotation, rects in expected.items():
        found = grid(z_rotation).find_rects(threshold=1000)

        # The largest rectangle comes first.
        if found[0][0:5] != rects[0]:
            return False

        if sorted(r[0:5] for r in found) != rects:
            return False

    return True

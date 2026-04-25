def unittest(data_path, temp_path):
    import image

    def make_radial(cx, cy):
        # 64x64 radial pattern centered at (cx, cy).
        img = image.Image(64, 64, image.GRAYSCALE)
        for y in range(64):
            for x in range(64):
                dx = x - cx
                dy = y - cy
                dist = int((dx * dx + dy * dy) ** 0.5)
                img.set_pixel((x, y), (dist * 8) % 256)
        return img

    # 1) Explicit center matching the image center matches the default.
    img1 = make_radial(32, 32)
    img1.logpolar(reverse=False)

    img2 = make_radial(32, 32)
    img2.logpolar(reverse=False, x=32, y=32)

    if img1.copy().difference(img2).get_statistics().l_max != 0:
        return False

    # 2) Off-center transform produces a valid image of the same size and
    #    differs from the default-centered transform of the same source.
    src = make_radial(20, 20)

    img3 = src.copy()
    img3.logpolar(reverse=False, x=20, y=20)
    if img3.width() != 64 or img3.height() != 64:
        return False

    img4 = src.copy()
    img4.logpolar(reverse=False)
    if img3.copy().difference(img4).get_statistics().l_max == 0:
        return False

    return True

# Image.draw_string(): every destination format, colours, alpha, hints, clipping,
# and the default font. Text is composited from the same coverage image Font.render()
# returns, so the checks compare against it.
def unittest(data_path, temp_path):
    import image

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    f = image.Font(data_path + "/font_mono.ttf", 20)
    text = "Agy1"
    w, h, base = f.measure(text)
    cov = f.render(text)

    def gray(color=255, **kw):
        img = image.Image(96, 48, image.GRAYSCALE)
        img.draw_string((4, 4), text, color=color, font=f, **kw)
        return img

    def count(img, thr=127):
        return sum(1 for y in range(img.height()) for x in range(img.width()) if img.get_pixel((x, y)) > thr)

    # GRAYSCALE: the drawn pixels are the coverage image at (x, y).
    g = gray()
    ok = all(abs(g.get_pixel((4 + x, 4 + y)) - cov.get_pixel((x, y))) <= 1 for y in range(h) for x in range(w))
    check("grayscale equals coverage", ok)
    check("nothing outside the box", all(g.get_pixel((x, y)) == 0 for y in range(48) for x in range(96)
                                         if not (4 <= x < 4 + w and 4 <= y < 4 + h)))
    check("returns the image", image.Image(8, 8, image.GRAYSCALE).draw_string((0, 0), "a", color=255, font=f) is not None)

    # Colour scales the coverage; black text on white works too.
    half = gray(color=128)
    check("gray colour", 120 <= half.get_statistics().max <= 130, str(half.get_statistics().max))
    wb = image.Image(96, 48, image.GRAYSCALE)
    wb.draw_rectangle((0, 0, 96, 48), color=255, fill=True)
    wb.draw_string((4, 4), text, color=0, font=f)
    check("black on white", wb.get_statistics().min == 0 and wb.get_pixel((95, 47)) == 255)

    # RGB565: colour channels arrive, and blend with the background by coverage.
    rgb = image.Image(96, 48, image.RGB565)
    rgb.draw_string((4, 4), text, color=(255, 0, 0), font=f)
    r, gr, b = max(rgb.get_pixel((x, y)) for y in range(48) for x in range(96))
    check("rgb red", r > 240 and gr < 16 and b < 16, str((r, gr, b)))
    check("rgb ink count", abs(count(rgb.to_grayscale(copy=True), 40) - count(cov, 127)) < count(cov, 127))
    rgb.draw_rectangle((0, 0, 96, 48), color=(0, 0, 255), fill=True)
    rgb.draw_string((4, 4), text, color=(255, 255, 0), font=f)
    px = set(rgb.get_pixel((x, y)) for y in range(48) for x in range(96))
    check("rgb over background", any(p[0] > 240 and p[2] < 16 for p in px) and any(p[2] > 240 and p[0] < 16 for p in px))
    check("rgb int colour", image.Image(32, 32, image.RGB565).draw_string((0, 0), "a", color=0xFFFF, font=f).get_statistics().l_max > 90)

    # A pixel inside the text is the colour asked for, in every format, however the colour
    # is given: it is the pixel draw_rectangle() makes of the same colour. Text is drawn
    # through an RGB565 colour palette, which holds a gray to within a few levels.
    def match(fmt, got, want):
        return (abs(got - want) <= 5) if fmt == image.GRAYSCALE else (got == want)

    sx, sy = [(x, y) for y in range(h) for x in range(w) if cov.get_pixel((x, y)) == 255][0]

    def inside(fmt, color, bg, **kw):
        img = image.Image(96, 48, fmt)
        img.draw_rectangle((0, 0, 96, 48), color=bg, fill=True)
        img.draw_string((4, 4), text, color=color, font=f, **kw)
        ref = image.Image(8, 8, fmt).draw_rectangle((0, 0, 8, 8), color=color, fill=True)
        return img.get_pixel((4 + sx, 4 + sy)), ref.get_pixel((1, 1))

    grays = list(range(0, 256, 5)) + [1, 2, 3, 4, 126, 127, 128, 129, 251, 252, 253, 254]
    wrong = [(c, inside(image.GRAYSCALE, c, 255 - c)[0]) for c in grays]
    wrong = [t for t in wrong if not match(image.GRAYSCALE, t[0], t[1])]
    check("grayscale colour", not wrong, str(wrong[:8]))
    for fmt, name, bg, colors in ((image.GRAYSCALE, "grayscale", 0, ((255, 255, 255), (128, 128, 128), (255, 0, 0), (0, 255, 0), (10, 20, 30))),
                                  (image.RGB565, "rgb565", (0, 0, 0), ((255, 255, 255), (255, 0, 0), (0, 255, 0), (0, 0, 255),
                                                                         (255, 220, 0), (18, 52, 86), 0xF800, 0x07E0, 0x1234)),
                                  (image.BINARY, "binary", 0, (1, 255, (255, 255, 255), (200, 200, 200)))):
        for color in colors:
            got, want = inside(fmt, color, bg)
            check("%s colour %s" % (name, color), match(fmt, got, want), "%s %s" % (got, want))
    check("binary dark colour", inside(image.BINARY, 0, 1) == (0, 0) and inside(image.BINARY, (0, 0, 0), 1) == (0, 0) and
          inside(image.BINARY, (100, 0, 0), 1) == (0, 0))
    # The outline's colour is as exact: two pixels left of the text is inside a 3 pixel outline.
    lx = min(x for x in range(w) if cov.get_pixel((x, sy)) > 0)

    def outline(fmt, color, bg, fill):
        img = image.Image(96, 48, fmt)
        img.draw_rectangle((0, 0, 96, 48), color=bg, fill=True)
        img.draw_string((4, 4), text, color=fill, font=f, stroke_width=3, stroke_color=color)
        ref = image.Image(8, 8, fmt).draw_rectangle((0, 0, 8, 8), color=color, fill=True)
        return img.get_pixel((4 + 3 + lx - 2, 4 + 3 + sy)), img.get_pixel((4 + 3 + sx, 4 + 3 + sy)), ref.get_pixel((1, 1))

    for fmt, name, bg, fill, colors in ((image.GRAYSCALE, "grayscale", 128, 255, (0, 1, 77, 200, 254, (255, 0, 0))),
                                        (image.RGB565, "rgb565", (128, 128, 128), (255, 255, 255), ((255, 0, 0), (0, 0, 255), (18, 52, 86), 0x07E0)),
                                        (image.BINARY, "binary", 1, 1, (0, (0, 0, 0)))):
        for color in colors:
            got, text_px, want = outline(fmt, color, bg, fill)
            check("%s stroke colour %s" % (name, color), match(fmt, got, want), "%s %s" % (got, want))
            check("%s text over stroke %s" % (name, color), match(fmt, text_px, inside(fmt, fill, bg)[1]), str(text_px))
    check("stroke colour defaults to black", outline(image.GRAYSCALE, None, 128, 255)[0] == 0)

    # BINARY: coverage is thresholded.
    bn = image.Image(96, 48, image.BINARY)
    bn.draw_string((4, 4), text, color=1, font=f)
    n = sum(1 for y in range(48) for x in range(96) if bn.get_pixel((x, y)))
    check("binary ink", 0 < n < w * h and abs(n - count(cov, 127)) < count(cov, 127) // 2, "%d %d" % (n, count(cov)))

    # Alpha: 0 draws nothing, 128 draws about half, 255 is the default.
    check("alpha 0", gray(alpha=0).get_statistics().max == 0)
    check("alpha 128", 120 <= gray(alpha=128).get_statistics().max <= 136, str(gray(alpha=128).get_statistics().max))
    check("alpha 255", gray(alpha=255).difference(g).get_statistics().max == 0)

    # scale is the same as a larger font.
    big = image.Image(96, 48, image.GRAYSCALE)
    big.draw_string((4, 4), "Ag", color=255, font=image.Font(data_path + "/font_mono.ttf", 30))
    sc = image.Image(96, 48, image.GRAYSCALE)
    sc.draw_string((4, 4), "Ag", color=255, font=f, scale=1.5)
    check("scale kwarg", sc.difference(big).get_statistics().max == 0)
    check("scale positional", image.Image(96, 48, image.GRAYSCALE).draw_string((4, 4), "Ag", 255, 1.5, font=f)
          .difference(big).get_statistics().max == 0)

    # Hints move pixels without changing how many there are.
    n0 = count(g)
    for name, hint in (("hmirror", image.HMIRROR), ("vflip", image.VFLIP), ("rotate 180", image.ROTATE_180)):
        m = gray(hint=hint)
        check(name + " ink", count(m) == n0, "%d %d" % (count(m), n0))
        check(name + " differs", m.difference(g).get_statistics().max > 0)
    hm = gray(hint=image.HMIRROR)
    # A GPU composites the unmirrored draw and may round one level away from the software loop.
    check("hmirror is a mirror", all(abs(hm.get_pixel((4 + w - 1 - x, 4 + y)) - g.get_pixel((4 + x, 4 + y))) <= 1
                                     for y in range(h) for x in range(w)))
    for name, hint in (("rotate 90", image.ROTATE_90), ("rotate 270", image.ROTATE_270), ("transpose", image.TRANSPOSE)):
        img = image.Image(64, 96, image.GRAYSCALE)
        img.draw_string((4, 4), text, color=255, font=f, hint=hint)
        check(name + " ink", count(img) == n0, "%d %d" % (count(img), n0))
        rows = [y for y in range(96) if any(img.get_pixel((x, y)) for x in range(64))]
        cols = [x for x in range(64) if any(img.get_pixel((x, y)) for y in range(96))]
        check(name + " is tall", (rows[-1] - rows[0]) > (cols[-1] - cols[0]))

    # Masked text is Font.render() plus draw_image(): the coverage is both the source and,
    # through an identity alpha palette, its own transparency. It must blend with the image
    # under the mask and leave everything else alone.
    mask = image.Image(96, 48, image.BINARY)
    mask.draw_rectangle((0, 0, 4 + w // 2, 48), color=1, fill=True)
    ramp = image.Image(256, 1, image.GRAYSCALE)
    for i in range(256):
        ramp.set_pixel((i, 0), i)
    ref = image.Image(96, 48, image.GRAYSCALE)
    ref.draw_rectangle((0, 0, 96, 48), color=60, fill=True)
    bg = ref.copy()
    ref.draw_image(cov, 4, 4, alpha_palette=ramp)
    bg.draw_image(cov, 4, 4, alpha_palette=ramp, mask=mask)
    check("unmasked reference draws", ref.get_statistics().max > 200)
    check("mask blends with the image", all(abs(bg.get_pixel((x, y)) - ref.get_pixel((x, y))) <= 1
                                           for y in range(48) for x in range(4 + w // 2)))
    check("mask leaves the rest", all(bg.get_pixel((x, y)) == 60 for y in range(48) for x in range(4 + w // 2, 96)))

    # Clipping: text that crosses or misses the image draws what is visible and nothing else.
    def at(x, y):
        img = image.Image(96, 48, image.GRAYSCALE)
        img.draw_string((x, y), text, color=255, font=f)
        return img
    check("clip left", 0 < count(at(-(w // 2), 4)) < n0)
    check("clip top", 0 < count(at(4, -(h // 2))) < n0)
    check("clip right", 0 < count(at(96 - w // 2, 4)) < n0)
    check("clip bottom", 0 < count(at(4, 48 - h // 2)) < n0)
    for name, pos in (("left", (-w - 8, 4)), ("right", (200, 4)), ("above", (4, -h - 8)), ("below", (4, 200))):
        check("fully off " + name, count(at(*pos)) == 0)
    part = at(-(w // 2), 4)
    check("clip keeps pixels", all(part.get_pixel((x, y)) == g.get_pixel((x + 4 + w // 2, y)) for y in range(48) for x in range(16)))

    # The default font draws with no font argument, and Font() is that font.
    d = image.Image(96, 24, image.GRAYSCALE)
    d.draw_string((2, 2), "default", color=255)
    e = image.Image(96, 24, image.GRAYSCALE)
    e.draw_string((2, 2), "default", color=255, font=image.Font())
    check("default font draws", count(d) > 20)
    check("default font is Font()", d.difference(e).get_statistics().max == 0)

    # Multi-line text reaches the image.
    ml = image.Image(96, 48, image.GRAYSCALE)
    ml.draw_string((2, 2), "ab\ncd", color=255, font=f)
    rows = [y for y in range(48) if any(ml.get_pixel((x, y)) for x in range(96))]
    check("multi-line", rows[-1] - rows[0] > f.line_height)

    # align moves the text against x: right aligned text ends where left aligned text
    # drawn its width to the left ends, and centered text straddles x.
    F = image.Font
    left = image.Image(96, 48, image.GRAYSCALE)
    left.draw_string((80 - w, 4), text, color=255, font=f)
    right = image.Image(96, 48, image.GRAYSCALE)
    right.draw_string((80, 4), text, color=255, font=f, align=F.RIGHT)
    check("align right", right.difference(left).get_statistics().max <= 1)
    mid = image.Image(96, 48, image.GRAYSCALE)
    mid.draw_string((48, 4), text, color=255, font=f, align=F.CENTER)
    cols = [x for x in range(96) if any(mid.get_pixel((x, y)) > 127 for y in range(48))]
    check("align center", abs((cols[0] + cols[-1]) / 2 - 48) <= 2, str(cols[:1] + cols[-1:]))
    # wrap_width breaks the line.
    wr = image.Image(96, 96, image.GRAYSCALE)
    wr.draw_string((4, 4), "Ag Ag", color=255, font=f, wrap_width=f.measure("Ag")[0] + 2)
    nl = image.Image(96, 96, image.GRAYSCALE)
    nl.draw_string((4, 4), "Ag\nAg", color=255, font=f)
    check("wrap_width", wr.difference(nl).get_statistics().max == 0 and count(nl) > count(g) // 2)
    left = right = mid = wr = nl = None

    # stroke_width outlines the text. The text is where it would be alone in a box grown
    # by the stroke; around it is the outline, and beyond that nothing is touched.
    def stroked(r, bg=128, **kw):
        img = image.Image(96, 48, image.GRAYSCALE)
        img.draw_rectangle((0, 0, 96, 48), color=bg, fill=True)
        img.draw_string((4, 4), text, color=255, font=f, stroke_width=r, stroke_color=0, **kw)
        return img

    check("stroke 0 is no stroke", stroked(0).difference(image.Image(96, 48, image.GRAYSCALE).draw_rectangle(
        (0, 0, 96, 48), color=128, fill=True).draw_string((4, 4), text, color=255, font=f)).get_statistics().max == 0)
    dark = []
    for r in (1, 2, 4):
        s = stroked(r)
        ok_fill = ok_out = ok_far = True
        n_dark = 0
        for y in range(48):
            for x in range(96):
                v = s.get_pixel((x, y))
                cx, cy = x - 4 - r, y - 4 - r
                c = cov.get_pixel((cx, cy)) if (0 <= cx < w and 0 <= cy < h) else 0
                if c >= 250:
                    ok_fill &= v >= 250
                elif c == 0:
                    ok_out &= v <= 129        # Never lighter than the background.
                    n_dark += v < 16
                inside = (4 <= x < 4 + w + 2 * r) and (4 <= y < 4 + h + 2 * r)
                if not inside:
                    ok_far &= v == 128
        check("stroke %d keeps the text" % r, ok_fill)
        check("stroke %d is the outline color" % r, ok_out and n_dark > 0, str(n_dark))
        check("stroke %d stays in its box" % r, ok_far)
        dark.append(n_dark)
    check("wider stroke covers more", dark[0] < dark[1] < dark[2], str(dark))
    # Every pixel within the stroke of solid text is solid outline or text.
    s = stroked(2)
    ok = True
    for y in range(h):
        for x in range(w):
            if cov.get_pixel((x, y)) >= 250:
                for dx, dy in ((-2, 0), (2, 0), (0, -2), (0, 2), (-1, -1), (1, 1)):
                    v = s.get_pixel((x + 4 + 2 + dx, y + 4 + 2 + dy))
                    cx, cy = x + dx, y + dy
                    c = cov.get_pixel((cx, cy)) if (0 <= cx < w and 0 <= cy < h) else 0
                    ok &= (v <= 2) if c == 0 else True
    check("stroke is solid next to the text", ok)
    # The outline takes the text's alpha, and both colors reach every format.
    half = stroked(2, alpha=128)
    full = stroked(2)
    check("stroke alpha", all(abs(half.get_pixel((x, y)) - (128 + full.get_pixel((x, y))) // 2) <= 3
                              for y in range(0, 48, 3) for x in range(0, 96, 3)))
    rgb = image.Image(96, 48, image.RGB565)
    rgb.draw_string((4, 4), text, color=(255, 255, 255), font=f, stroke_width=2, stroke_color=(255, 0, 0))
    red = white = 0
    between = True
    for y in range(48):
        for x in range(96):
            p = rgb.get_pixel((x, y))
            red += p[0] > 240 and p[1] < 12 and p[2] < 12
            white += min(p) > 240
            # Every pixel is black, or somewhere from red to white: green and blue rise together.
            between &= (p == (0, 0, 0)) or (p[0] >= p[1] - 8 and abs(p[1] - p[2]) <= 12)
    check("stroke rgb565", red > 0 and white > 0 and between, "%d %d %s" % (red, white, between))
    rgb = half = full = s = None
    binary = image.Image(96, 48, image.BINARY)
    binary.draw_rectangle((0, 0, 96, 48), color=1, fill=True)
    binary.draw_string((4, 4), text, color=1, font=f, stroke_width=2, stroke_color=0)
    nb = sum(1 for y in range(48) for x in range(96) if not binary.get_pixel((x, y)))
    check("stroke binary", 0 < nb < (w + 4) * (h + 4))
    # Outlined and plain glyphs are cached apart: plain text is the same after outlined text.
    check("stroke cached apart", gray().difference(g).get_statistics().max == 0)
    before = image.Font.cache_info()
    stroked(2)
    after = image.Font.cache_info()
    if after.maxsize:
        check("stroke cached", after.hits > before.hits and after.misses == before.misses, "%s %s" % (before, after))

    # Argument errors.
    for name, kw, exc in (("font type", {"font": "mono"}, TypeError),
                          ("align", {"align": 3}, ValueError),
                          ("wrap_width", {"wrap_width": -1}, ValueError),
                          ("stroke_width negative", {"stroke_width": -1}, ValueError),
                          ("stroke_width too wide", {"stroke_width": 33}, ValueError),
                          ("stroke_color", {"stroke_width": 1, "stroke_color": "red"}, TypeError), ("scale 0", {"scale": 0}, ValueError), ("scale negative", {"scale": -1}, ValueError),
                          ("removed mono_space", {"mono_space": False}, TypeError),
                          ("removed x_spacing", {"x_spacing": 2}, TypeError),
                          ("removed y_spacing", {"y_spacing": 2}, TypeError),
                          ("no mask argument", {"mask": image.Image(32, 32, image.BINARY)}, TypeError),
                          ("removed string_rotation", {"string_rotation": 90}, TypeError)):
        try:
            image.Image(32, 32, image.GRAYSCALE).draw_string((0, 0), "x", color=255, **kw)
            check(name, False, "no exception")
        except exc:
            pass
        except Exception as ex:
            check(name, False, repr(ex))

    # Repeated calls do not leak scratch memory (the runner checks the allocator afterwards).
    img = image.Image(96, 48, image.RGB565)
    for i in range(50):
        img.draw_string((i % 20 - 10, i % 30 - 10), "leak %d\ncheck" % i, color=(255, 255, 255), font=f, alpha=200)
    return not fails

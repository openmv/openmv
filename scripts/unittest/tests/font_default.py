# The default font keeps the footprint of the 8x10 bitmap font it replaced, so scripts
# written for that font still lay out: draw_string() with no font draws every printable
# ASCII character inside an 8x10 cell at (x, y), and lines are 10 pixels apart.
def unittest(data_path, temp_path):
    import image

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    d = image.Font()
    check("size", d.size == 10)
    check("line height", d.line_height == 10, str(d.line_height))
    check("baseline", d.ascender == 8 and d.descender == -2, "%d %d" % (d.ascender, d.descender))

    # One line is 10 pixels tall, n lines are 10 * n, and text is never wider than 8 per character.
    for n in (1, 2, 5, 20, 40):
        w, h, b = d.measure("W" * n)
        check("width %d" % n, w <= 8 * n, "%d" % w)
        check("height %d" % n, h == 10 and b == 8, "%d %d" % (h, b))
    # (Descenders reach a fraction of a pixel below the last line's cell.)
    check("two lines", 20 <= d.measure("Ag\nAg")[1] <= 21, str(d.measure("Ag\nAg")))
    check("five lines", d.measure("a\nb\nc\nd\ne")[1] == 50)

    # Every printable ASCII glyph stays inside the 8x10 cell at the drawing position.
    outside = []
    for c in range(0x21, 0x7F):
        img = image.Image(40, 40, image.GRAYSCALE)
        img.draw_string((16, 16), chr(c), color=255)
        for y in range(40):
            for x in range(40):
                if img.get_pixel((x, y)) > 40 and not (16 <= x < 24 and 16 <= y < 26):
                    outside.append((chr(c), x - 16, y - 16))
    check("glyphs inside the 8x10 cell", not outside, repr(outside[:6]))

    # A string stays inside the box the old font used for it: 8 * len wide, 10 tall.
    text = "The quick brown fox (jumps) over {lazy} dog_|"
    img = image.Image(8 * len(text) + 32, 42, image.GRAYSCALE)
    img.draw_string((16, 16), text, color=255)
    spill = [(x, y) for y in range(img.height()) for x in range(img.width())
             if img.get_pixel((x, y)) > 40 and not (16 <= x < 16 + 8 * len(text) and 16 <= y < 26)]
    check("string inside the old box", not spill, repr(spill[:6]))
    check("string draws", img.get_statistics().max > 150, str(img.get_statistics().max))

    # scale=2 fills a 16x20 cell the same way; descenders may shade one row below it.
    img = image.Image(96, 64, image.GRAYSCALE)
    img.draw_string((16, 16), "Agjy|", color=255, scale=2)
    spill = [(x, y) for y in range(64) for x in range(96)
             if img.get_pixel((x, y)) > 40 and not (16 <= x < 16 + 16 * 5 and 16 <= y < 37)]
    check("scale 2 inside 16x20 cells", not spill, repr(spill[:6]))
    check("scale 2 no solid spill", all(img.get_pixel((x, 36)) < 128 for x in range(96)))

    # Multi-line text advances 10 pixels per line.
    img = image.Image(64, 48, image.GRAYSCALE)
    img.draw_string((4, 4), "H\nH\nH", color=255)
    rows = [y for y in range(48) if any(img.get_pixel((x, y)) > 127 for x in range(64))]
    tops = [y for y in rows if (y - 1) not in rows]
    check("line pitch", len(tops) == 3 and tops[1] - tops[0] == 10 and tops[2] - tops[1] == 10, repr(tops))
    return not fails

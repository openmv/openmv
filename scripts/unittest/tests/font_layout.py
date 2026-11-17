# Text layout: Font.measure() and Font.render() agree, scale re-rasterizes, line breaks
# move the pen as documented, and UTF-8 is decoded leniently.
def unittest(data_path, temp_path):
    import image

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    def ink(img):
        return sum(1 for y in range(img.height()) for x in range(img.width()) if img.get_pixel((x, y)) > 127)

    def same(a, b):
        return a.width() == b.width() and a.height() == b.height() and a.copy().difference(b).get_statistics().max == 0

    path = data_path + "/font_mono.ttf"
    f = image.Font(path, 16, fallback=False)
    prop = image.Font(data_path + "/font_composite.ttf", 16)

    # measure() describes exactly the image render() returns.
    for text in ("A", "Hello, World!", "gjpqy", "line one\nline two", "  padded  ", "|"):
        w, h, b = f.measure(text)
        img = f.render(text)
        check("measure==render %r" % text, (img.width(), img.height()) == (w, h), str((w, h)))
        check("format %r" % text, img.format() == image.GRAYSCALE)
        check("baseline %r" % text, 0 < b <= h)

    # Nothing to draw.
    check("empty measure", f.measure("")[0] == 0)
    e = f.render("")
    check("empty render", e.width() >= 1 and ink(e) == 0)
    check("empty lines render", ink(f.render("\n\n")) == 0 and f.render("\n\n").height() == 3 * f.line_height,
          "%d %d" % (f.render("\n\n").height(), f.line_height))
    check("spaces have width, no ink", f.measure("   ")[0] > 0 and ink(f.render("   ")) == 0)

    # Fixed pitch: every glyph advances the same amount; a proportional font does not.
    adv = f.measure("0")[0]
    check("mono advances", all(f.measure(c)[0] == adv for c in "iW.m1"), str([f.measure(c)[0] for c in "iW.m1"]))
    # (The advance is fractional, so compare long runs.)
    step10 = f.measure("0" * 20)[0] - f.measure("0" * 10)[0]
    check("mono string width", abs(f.measure("0" * 30)[0] - f.measure("0" * 10)[0] - 2 * step10) <= 1)
    check("mono run widths", f.measure("i" * 20)[0] == f.measure("W" * 20)[0] or
          abs(f.measure("i" * 20)[0] - f.measure("W" * 20)[0]) <= 2)
    check("proportional advances", prop.measure("i")[0] < prop.measure("W")[0])

    # scale multiplies the size and re-rasterizes: Font(16) at scale 2 is Font(32) at scale 1.
    f32 = image.Font(path, 32)
    check("scale measure", f.measure("Scale", scale=2) == f32.measure("Scale"))
    check("scale render", same(f.render("Scale", scale=2), f32.render("Scale")))
    check("scale fractional", same(f.render("Half", scale=0.5), image.Font(path, 8).render("Half")))
    check("scale grows", f.measure("x", scale=3)[0] > f.measure("x", scale=1.5)[0] > f.measure("x")[0])
    for bad in (0, -1):
        try:
            f.measure("x", scale=bad)
            check("scale %d raises" % bad, False)
        except ValueError:
            pass

    # Lines: \n, \r\n and \r all break; height grows by the line height; width is the widest line.
    one = f.measure("abc")
    for sep in ("\n", "\r\n", "\r"):
        two = f.measure("abc" + sep + "abcdef")
        check("newline %r height" % sep, two[1] - one[1] == f.line_height, "%d %d %d" % (two[1], one[1], f.line_height))
        check("newline %r width" % sep, two[0] == f.measure("abcdef")[0])
        check("newline %r baseline" % sep, two[2] == one[2])
    check("blank lines", f.measure("a\n\n\nb")[1] - one[1] == 3 * f.line_height)
    check("trailing newline", f.measure("abc\n")[1] - one[1] == f.line_height)
    # The second line is drawn below the first: ink appears in both halves.
    img = f.render("X\nX")
    top = sum(1 for y in range(img.height() // 2) for x in range(img.width()) if img.get_pixel((x, y)) > 127)
    check("two lines ink", 0 < top < ink(img) and abs(2 * top - ink(img)) <= 4, "%d %d" % (top, ink(img)))

    # Baselines are on whole pixels: n lines are exactly n line heights tall, the baseline is
    # exactly the ascender, and every line of the same text is rasterized identically. This
    # font's own metrics are fractional at these sizes.
    for px in (11, 13, 16, 19, 23.5):
        g = image.Font(path, px, fallback=False)
        lh = g.line_height
        for n in (1, 2, 3, 7):
            check("exact height %s x%d" % (px, n), g.measure("\n".join(["x"] * n))[1] == n * lh,
                  "%d %d" % (g.measure("\n".join(["x"] * n))[1], n * lh))
        check("exact baseline %s" % px, g.measure("x")[2] == g.ascender, "%d %d" % (g.measure("x")[2], g.ascender))
        # Compare two inner lines: both have a neighbour above and below, so any ink that
        # crosses a line boundary (descenders, tall brackets) is the same for each.
        img = g.render("Hxg\nHxg\nHxg\nHxg")
        w = img.width()
        rows = [img.copy(roi=(0, i * lh, w, lh)) for i in (1, 2)]
        check("lines identical %s" % px, rows[0].difference(rows[1]).get_statistics().max == 0)

    # Control characters other than line breaks are skipped.
    check("tab skipped", f.measure("a\tb") == f.measure("ab"))
    check("bell skipped", f.measure("a\x07b") == f.measure("ab"))

    # UTF-8: multi-byte sequences are one character each.
    check("2-byte", prop.measure("é")[0] == prop.measure("e")[0])
    # (Thai is in no shipped font, so these are the font's own .notdef box.)
    check("3-byte is one glyph", f.measure("ก")[0] == f.measure("ข")[0] == adv)
    check("4-byte is one glyph", f.measure("\U0001F600")[0] == adv)

    # Alignment says which part of each line is at the origin. The box is the same size
    # whatever the alignment, and measure() still describes what render() returns.
    F = image.Font
    lh = f.line_height
    av = f.measure("0" * 40)[0] / 40     # The advance of this fixed pitch font.
    w0, h0, b0 = f.measure("Ag\nxy")
    check("align constants", (F.LEFT, F.CENTER, F.RIGHT) == (0, 1, 2) and f.RIGHT == F.RIGHT)
    two = "ab\nabcdefgh"
    w, h, b = f.measure(two)
    for name, align in (("left", F.LEFT), ("center", F.CENTER), ("right", F.RIGHT)):
        img = f.render(two, align=align)
        check("align %s size" % name, (img.width(), img.height()) == f.measure(two, align=align)[:2] and
              abs(img.width() - w) <= 1 and img.height() == h, str((img.width(), img.height(), w, h)))
        # The long line fills the box; the short one is at the left, in the middle or at the right.
        cols = [x for x in range(img.width()) if any(img.get_pixel((x, y)) > 127 for y in range(lh))]
        mid = (cols[0] + cols[-1]) / 2
        want = {F.LEFT: av, F.CENTER: img.width() / 2, F.RIGHT: img.width() - av}[align]
        check("align %s position" % name, abs(mid - want) <= 2, "%s %s" % (mid, want))
    # One line is the same picture however it is aligned.
    check("align one line", same(f.render("abc"), f.render("abc", align=F.RIGHT)) and
          same(f.render("abcd"), f.render("abcd", align=F.CENTER)))
    # Each line of aligned text is the line on its own.
    r2 = f.render(two, align=F.RIGHT)
    one = f.render("ab")
    check("right aligned line", same(r2.copy(roi=(r2.width() - one.width(), 0, one.width(), lh)), one.copy(roi=(0, 0, one.width(), lh))))

    # wrap_width breaks lines at spaces, exactly as a line break in the same place would.
    def wrapped(text, chars, lines, name, **kw):
        ww = int(av * chars) + 1
        img = f.render(text, wrap_width=ww, **kw)
        check(name, same(img, f.render(lines, **kw)), "%s vs %s" % (f.measure(text, wrap_width=ww, **kw), f.measure(lines, **kw)))
        check(name + " fits", img.width() <= max(ww, int(av) + 1) + 1, "%d > %d" % (img.width(), ww))
        check(name + " measure", f.measure(text, wrap_width=ww, **kw)[:2] == (img.width(), img.height()))

    wrapped("aaa bbb ccc", 7, "aaa bbb\nccc", "wrap at space")
    wrapped("aaa bbb ccc", 3, "aaa\nbbb\nccc", "wrap every word")
    wrapped("aaa bbb ccc", 100, "aaa bbb ccc", "wrap not needed")
    wrapped("aaa   bbb", 4, "aaa\nbbb", "wrap drops a run of spaces")
    wrapped("aaa bbb", 6, "aaa\nbbb", "wrap space does not fit")
    # A word wider than the limit is broken wherever it stops fitting.
    wrapped("abcdefgh", 4, "abcd\nefgh", "wrap long word")
    wrapped("ab abcdefgh", 4, "ab\nabcd\nefgh", "wrap long word after a word")
    wrapped("abc", 0.5, "a\nb\nc", "wrap narrower than a character")
    # Line breaks in the text still break, and aligned lines wrap the same way.
    wrapped("aa bb\ncc dd", 2, "aa\nbb\ncc\ndd", "wrap with line breaks")
    wrapped("aaa bbb c", 7, "aaa bbb\nc", "wrap centered", align=F.CENTER)
    wrapped("aaa bbb c", 7, "aaa bbb\nc", "wrap right", align=F.RIGHT)
    check("wrap empty", f.measure("", wrap_width=20) == f.measure(""))
    check("wrap height", f.measure("a b c d", wrap_width=int(av) + 1)[1] == 4 * lh)

    # The outline is part of the box: it grows by the stroke on every side.
    for stroke in (1, 3):
        check("stroke %d box" % stroke, f.measure("Ag\nxy", stroke_width=stroke) ==
              (w0 + 2 * stroke, h0 + 2 * stroke, b0 + stroke), str(f.measure("Ag\nxy", stroke_width=stroke)))
        ww = int(av * 4) + 1 + 2 * stroke
        check("stroke %d wraps inside" % stroke, f.measure("aaaa bbbb", wrap_width=ww, stroke_width=stroke)[0] <= ww + 1 and
              f.measure("aaaa bbbb", wrap_width=ww, stroke_width=stroke)[1] == 2 * lh + 2 * stroke)

    # Argument errors.
    for name, fn, exc in (("align range", lambda: f.measure("a", align=3), ValueError),
                          ("align negative", lambda: f.render("a", align=-1), ValueError),
                          ("wrap negative", lambda: f.measure("a", wrap_width=-1), ValueError),
                          ("stroke negative", lambda: f.measure("a", stroke_width=-1), ValueError),
                          ("stroke too wide", lambda: f.measure("a", stroke_width=33), ValueError),
                          ("render has no stroke", lambda: f.render("a", stroke_width=1), TypeError)):
        try:
            fn()
            check(name, False, "no exception")
        except exc:
            pass
        except Exception as ex:
            check(name, False, repr(ex))

    # Large and tiny sizes.
    big = image.Font(path, 96).render("Ag")
    check("large size", big.height() > 90 and ink(big) > 1000)
    tiny = image.Font(path, 4).render("tiny")
    check("tiny size", tiny.width() > 0)
    return not fails

# Arabic shaping and fallback fonts. Shaping picks each letter's contextual form from the
# font's Unicode presentation forms, so the shaped string must render exactly like the
# same presentation-form code points written out by hand in visual (left to right) order.
def unittest(data_path, temp_path):
    import image, os

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    def same(a, b):
        return a.width() == b.width() and a.height() == b.height() and a.copy().difference(b).get_statistics().max == 0

    def ink(img):
        return sum(1 for y in range(img.height()) for x in range(img.width()) if img.get_pixel((x, y)) > 127)

    ar = image.Font(data_path + "/font_arabic.ttf", 28, fallback=image.Font(data_path + "/font_mono.ttf", 28))
    mono = image.Font(data_path + "/font_mono.ttf", 28)
    # fallback=False: only this font, so what it lacks is its .notdef box on every board.
    alone = image.Font(data_path + "/font_mono.ttf", 28, fallback=False)

    def shaped(logical, visual, name):
        check(name, same(ar.render(logical), ar.render(visual)),
              "%s vs %s" % (ar.measure(logical), ar.measure(visual)))

    # The expected strings spell out the presentation forms in logical order; form code
    # points do not join, so the shaper only reverses them, exactly as it does the input.
    # One letter alone takes its isolated form.
    shaped("ب", "ﺏ", "isolated beh")
    # Three joined letters: initial, medial, final.
    shaped("ببب", "ﺑﺒﺐ", "initial medial final")
    shaped("بب", "ﺑﺐ", "initial final")
    # Alef joins only to the letter before it, so the letter after it starts over.
    shaped("باب", "ﺑﺎﺏ", "right joining alef")
    # Dal and reh also break the join after them.
    shaped("دب", "ﺩﺏ", "dal does not join forward")
    shaped("برب", "ﺑﺮﺏ", "reh final then isolated")
    # Hamza joins nothing.
    shaped("بءب", "ﺏﺀﺏ", "hamza is non joining")
    # Lam + alef is one ligature, isolated or final, for each alef variant.
    shaped("لا", "ﻻ", "lam alef isolated")
    shaped("بلا", "ﺑﻼ", "lam alef final")
    shaped("لأ", "ﻷ", "lam alef hamza")
    shaped("لآ", "ﻵ", "lam alef madda")
    shaped("لإ", "ﻹ", "lam alef hamza below")
    # Alef maksura and teh marbuta take final forms.
    shaped("بى", "ﺑﻰ", "alef maksura final")
    shaped("بة", "ﺑﺔ", "teh marbuta final")
    # Tatweel joins on both sides.
    shaped("بـب", "ﺑـﺐ", "tatweel")
    # A space separates words: each shapes on its own.
    shaped("بب سس", "ﺑﺐ ﺳﺲ", "two words")
    # A real word, salam: seen (initial), lam-alef (final), meem (isolated).
    shaped("سلام", "ﺳﻼﻡ", "salam")

    # Wrapped Arabic breaks in reading order: the first words stay on the first line.
    ww = ar.measure("سس")[0] + 3
    check("arabic wrap", same(ar.render("بب سس", wrap_width=ww), ar.render("بب\nسس")),
          "%s vs %s" % (ar.measure("بب سس", wrap_width=ww), ar.measure("بب\nسس")))
    check("arabic wrap right", same(ar.render("بب سس", wrap_width=ww, align=image.Font.RIGHT),
                                    ar.render("بب\nسس", align=image.Font.RIGHT)))

    # Marks are transparent to joining, take no advance and are drawn (more ink than without).
    check("mark has no advance", ar.measure("بَب")[0] == ar.measure("بب")[0])
    check("mark draws", ink(ar.render("بَب")) > ink(ar.render("بب")))
    # With the mark between them the two letters still take their joined forms.
    shaped("بَب", "ﺑَﺐ", "mark keeps the join")
    check("several marks", ar.measure("بَّ")[0] == ar.measure("ب")[0])

    # Numbers inside Arabic keep their left to right order; the words around them swap.
    shaped("ب 12 س", "ﺏ 12 ﺱ", "number between words")
    shaped("ب 3.5 س", "ﺏ 3.5 ﺱ", "decimal number")
    check("digits not mirrored", not same(ar.render("ب 12 س"), ar.render("ب 21 س")))
    # Arabic-Indic digits are in the font and read left to right too.
    shaped("ب ١٢", "ﺏ ١٢", "arabic-indic digits")

    # Latin around Arabic stays where it was; only the Arabic run is reversed.
    shaped("ab بب cd", "ab ﺑﺐ cd", "latin arabic latin")
    shaped("xب", "xﺏ", "latin then arabic")
    # Latin-only text is untouched by the shaper.
    check("latin unchanged", ar.measure("Hello123")[0] == mono.measure("Hello123")[0])
    check("visual order", not same(ar.render("بس"), ar.render("سب")))

    # Each line is shaped on its own.
    two = ar.render("بب\nب")
    check("lines shaped separately", two.height() > ar.render("بب").height() and ink(two) > ink(ar.render("بب")))

    # A font with no Arabic at all draws boxes, never crashes, and keeps one box per letter.
    w = alone.measure("ب")[0]
    check("no arabic font", alone.measure("سلم")[0] == 3 * w, "%d %d" % (alone.measure("سلم")[0], w))
    # A font with the letters but not their presentation forms falls back to the plain letters.
    check("unshaped fallback", ink(ar.render("١")) > 0)

    # draw_string() goes through the same path.
    img = image.Image(160, 48, image.GRAYSCALE)
    img.draw_string((2, 2), "سلام 12", color=255, font=ar)
    check("draw_string arabic", ink(img) > 100)

    # ---- Fallback fonts ----
    cjk = image.Font(data_path + "/font_cjk.ttf", 28)
    box = alone.render("中")
    # Without a fallback to a font that has it, the default chain is tried, so compare
    # against an explicit chain instead of asserting a box.
    chain = image.Font(data_path + "/font_mono.ttf", 28, fallback=cjk)
    check("fallback attribute", chain.fallback is cjk)
    check("fallback False attribute", alone.fallback is False)
    check("fallback False draws a box", same(alone.render("中"), alone.render("文")) and not same(alone.render("中"), alone.render("A")))
    def close(a, b):
        return abs(a - b) <= max(a, b) // 6

    # (The glyph sits on the primary font's baseline, at a different sub-pixel offset than in
    # the fallback font's own box, so ink is compared loosely and widths exactly.)
    check("fallback draws the glyph", close(ink(chain.render("中")), ink(cjk.render("中"))) and
          chain.measure("中")[0] == cjk.measure("中")[0])
    check("fallback not a box", chain.measure("中")[0] != box.width() or not close(ink(chain.render("中")), ink(box)))
    check("primary font wins", same(chain.render("A"), mono.render("A")))
    # The primary font's metrics set the line; the fallback glyph is scaled to the same size.
    check("mixed width", chain.measure("A中A")[0] == 2 * mono.measure("A")[0] + cjk.measure("中")[0],
          "%s %s %s" % (chain.measure("A中A"), mono.measure("A"), cjk.measure("中")))
    check("mixed line height", chain.line_height == mono.line_height)
    # Chains: mono -> arabic -> cjk reaches the third font.
    deep = image.Font(data_path + "/font_mono.ttf", 28,
                      fallback=image.Font(data_path + "/font_arabic.ttf", 28, fallback=cjk))
    check("three font chain", ink(deep.render("A")) > 0 and ink(deep.render("ب")) > 0 and
          close(ink(deep.render("中")), ink(cjk.render("中"))))
    check("chain shapes arabic", same(deep.render("بب"), deep.render("ﺑﺐ")))
    # A character no font has is the first font's box.
    check("nobody has it", same(deep.render("ก"), alone.render("ก")))
    # The fallback is kept alive by the font that uses it.
    import gc
    keep = image.Font(data_path + "/font_mono.ttf", 28, fallback=image.Font(data_path + "/font_cjk.ttf", 28))
    gc.collect()
    check("fallback kept alive", close(ink(keep.render("中")), ink(cjk.render("中"))))

    # has(): can this font, with its fallbacks, draw the text without a box?
    check("has own glyphs", alone.has("Hello, World! 123"))
    check("has empty", alone.has("") and alone.has("\n\t"))
    check("has not", not alone.has("中") and not alone.has("ab中c"))
    check("has through fallback", chain.has("A中") and not chain.has("ก"))
    check("has through chain", deep.has("Aب中") and not deep.has("ก"))
    check("has arabic", deep.has("بب"))
    try:
        alone.has(123)
        check("has type", False)
    except TypeError:
        pass

    # The default font: Font() is it, and /rom/fonts.txt (when present) names it and its chain.
    d = image.Font()
    check("default has no explicit fallback", d.fallback is None)
    try:
        names = [n for n in open("/rom/fonts.txt").read().split("\n") if n]
    except OSError:
        names = []
    if names:
        first = image.Font("/rom/" + names[0], d.size)
        check("fonts.txt default", first.glyphs == d.glyphs and same(first.render("Default"), d.render("Default")))
        for n in names[1:]:
            check("fonts.txt fallback " + n, n in os.listdir("/rom"))
    # A font with no explicit fallback still draws what the default font can.
    check("implicit default fallback", ink(image.Font(data_path + "/font_cjk.ttf", 20).render("z")) > 0)
    return not fails

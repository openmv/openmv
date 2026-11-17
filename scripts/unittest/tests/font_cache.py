# The glyph cache: rendered glyphs are kept and reused, text drawn from the cache is pixel
# for pixel what rasterizing it gives, glyphs in use survive when the cache fills, and a
# font that goes away takes its glyphs with it. Font.cache_info() reports the counters.
def unittest(data_path, temp_path):
    import image, gc

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    info = image.Font.cache_info
    if info().maxsize == 0:
        return "skip"                               # No cache on this board.

    def same(a, b):
        # (difference() works in place, so compare a copy.)
        return a.width() == b.width() and a.height() == b.height() and a.copy().difference(b).get_statistics().max == 0

    def delta(before, key):
        return getattr(info(), key) - getattr(before, key)

    path = data_path + "/font_mono.ttf"
    # A font in ROMFS is one set of bytes however many Font objects use it. Loaded from a
    # filesystem, each object has its own copy in RAM and so its own glyphs.
    rom = data_path.startswith("/rom")
    # Start from an empty cache so the counts below do not depend on earlier tests.
    image.Font.cache_clear()
    i = info()
    check("info fields", (i.hits, i.misses, i.currsize, i.glyphs, i.evictions) == (0, 0, 0, 0, 0) and i.maxsize > 0, str(i))
    check("info is a tuple", tuple(i) == (i.hits, i.misses, i.maxsize, i.currsize, i.glyphs, i.evictions))

    # A size nothing else uses, at which this fixed pitch font advances a whole number of
    # pixels (0.6 em), so every glyph is at the same sub-pixel offset: the first draw misses
    # once per distinct glyph, the second hits.
    f = image.Font(path, 15, fallback=False)
    text = "cache me if you can"
    distinct = len(set(text) - {" "})
    b = info()
    first = f.render(text)
    check("first draw misses", delta(b, "misses") == distinct, "%d %d" % (delta(b, "misses"), distinct))
    check("repeats within a string hit", delta(b, "hits") == len(text.replace(" ", "")) - distinct, str(delta(b, "hits")))
    check("glyphs stored", delta(b, "glyphs") == distinct and 0 < info().currsize <= info().maxsize)
    b = info()
    second = f.render(text)
    check("second draw hits", delta(b, "hits") == len(text.replace(" ", "")) and delta(b, "misses") == 0,
          "%d %d" % (delta(b, "hits"), delta(b, "misses")))
    check("cached is identical", same(first, second))
    check("nothing added", delta(b, "glyphs") == 0 and delta(b, "currsize") == 0)

    # measure() does not rasterize, so it does not touch the cache.
    b = info()
    f.measure("measure only XYZ")
    check("measure leaves the cache alone", delta(b, "hits") == 0 and delta(b, "misses") == 0)

    # The key is the final pixel size: scale and a font of that size share entries, and another
    # font object for the same file shares them too.
    f.render("scale")
    b = info()
    image.Font(path, 30, fallback=False).render("scale")
    f.render("scale", scale=2)
    if rom:
        check("scale shares with size", delta(b, "misses") == len(set("scale")), str(delta(b, "misses")))
        b = info()
        image.Font(path, 15, fallback=False).render(text)
        check("font objects share", delta(b, "misses") == 0)
    b = info()
    f.render("scale", scale=2)
    check("scale is cached", delta(b, "misses") == 0 and delta(b, "hits") == 5)
    # A different font file with the same glyph ids does not.
    b = info()
    image.Font(data_path + "/font_composite.ttf", 15, fallback=False).render("cache")
    check("fonts are separate", delta(b, "misses") >= len(set("cache")), str(delta(b, "misses")))

    # draw_string() uses the cache too, and draws the same pixels on a hit as on a miss.
    g = image.Font(path, 25, fallback=False)
    a = image.Image(160, 40, image.GRAYSCALE)
    b = info()
    a.draw_string((3, 3), "Draw 123", color=255, font=g)
    check("draw_string misses", delta(b, "misses") == len(set("Draw123")))
    # Drawn again it comes from the cache. (The cache may be nearly full of other tests'
    # glyphs, in which case making room for these can take a draw or two to settle.)
    for attempt in range(4):
        c = image.Image(160, 40, image.GRAYSCALE)
        b = info()
        c.draw_string((3, 3), "Draw 123", color=255, font=g)
        check("draw_string identical %d" % attempt, same(a, c))
        if delta(b, "misses") == 0:
            break
    check("draw_string hits", delta(b, "misses") == 0 and delta(b, "hits") == 7,
          "%d %d" % (delta(b, "misses"), delta(b, "hits")))

    # A proportional font puts glyphs at quarter pixel offsets; each is cached and the
    # result is still stable from one draw to the next.
    pf = image.Font(data_path + "/font_composite.ttf", 19, fallback=False)
    r1 = pf.render("Proportional illumination")
    check("proportional stable", same(r1, pf.render("Proportional illumination")))
    pf.render("illumi")
    b = info()
    pf.render("illumi")
    check("proportional cached", delta(b, "misses") == 0 and delta(b, "hits") == 6)

    # A glyph that would take more than a quarter of the cache is drawn directly, every
    # time. The size is chosen from the cache's so that a "W" (about 0.6 x 0.7 em) is just
    # over that, which keeps the rendered image small on every board.
    big = image.Font(path, int((info().maxsize / 4 / 0.4) ** 0.5) + 10, fallback=False)
    b = info()
    x1 = big.render("W")
    x2 = big.render("W")
    check("oversize bypasses", delta(b, "glyphs") == 0 and delta(b, "hits") == 0 and same(x1, x2))

    # Fill the cache several times over with glyphs used once, a couple per round, while one
    # string is drawn every round. Eviction must happen, the cache must stay within its size,
    # and the string in use must stay cached (second chance) and keep rendering identically.
    hot = image.Font(path, 14, fallback=False)
    cold = image.Font(path, 16, fallback=False)
    hot_text = "FPS 29.97"
    ref = hot.render(hot_text)
    hot_missed = 0
    b0 = info()
    rounds = 0
    while (info().evictions - b0.evictions) < 200 and rounds < 1000:
        # (A size and letters not used before, so these are never hits. One font object at
        # varying scales: a font loaded from a filesystem is purged when it is deleted.)
        cold.render("ABCDEFGHIJKLMNOPQRSTUVWXYZ"[(rounds // 160) * 2:(rounds // 160) * 2 + 2],
                    scale=(18 + (rounds % 160) / 16) / 16)
        b = info()
        check_img = hot.render(hot_text)
        if rounds >= 8:
            # (A glyph cached moments ago has not been used a second time yet, so it can
            # be dropped once while the cache is full of older glyphs that have. After that
            # it is in use and must stay.)
            hot_missed += delta(b, "misses")
        if not same(check_img, ref):
            check("hot text stable under eviction", False, "round %d" % rounds)
            break
        rounds += 1
    check("eviction happened", info().evictions - b0.evictions >= 200, str(info().evictions - b0.evictions))
    check("cache stays within its size", info().currsize <= info().maxsize)
    check("hot glyphs survive", hot_missed == 0, "re-rasterized %d times in %d rounds" % (hot_missed, rounds))
    check("glyph count sane", 0 < info().glyphs < info().maxsize // 20)

    # A font copied into RAM takes its glyphs with it when it is deleted, so a font loaded
    # later at the same address cannot pick them up.
    tmp = temp_path + "/font_cache.ttf"
    with open(tmp, "wb") as fp:
        fp.write(open(path, "rb").read())
    ram = image.Font(tmp, 23, fallback=False)
    ram.render("purge")
    g0 = info().glyphs
    del ram
    gc.collect()
    check("deleted font is purged", g0 - info().glyphs >= len(set("purge")), "%d %d" % (g0, info().glyphs))
    # The same bytes loaded again start from misses, and a different font's bytes at whatever
    # address render as that font.
    with open(tmp, "wb") as fp:
        fp.write(open(data_path + "/font_composite.ttf", "rb").read())
    other = image.Font(tmp, 23, fallback=False)
    check("no stale glyphs", same(other.render("purge"), image.Font(data_path + "/font_composite.ttf", 23, fallback=False).render("purge")))

    # cache_clear() empties it; the next draw starts over and still renders the same.
    again = image.Font(path, 15, fallback=False)
    before_clear = again.render(text)
    image.Font.cache_clear()
    i = info()
    check("clear empties", (i.hits, i.misses, i.currsize, i.glyphs) == (0, 0, 0, 0))
    check("same after clear", same(again.render(text), before_clear) and info().misses == distinct)

    # ROMFS fonts are not purged when a Font object for them is deleted.
    k = image.Font(path, 26, fallback=False)
    k.render("keep")
    del k
    gc.collect()
    b = info()
    image.Font(path, 26, fallback=False).render("keep")
    if rom:
        check("rom font glyphs kept", delta(b, "misses") == 0, str(delta(b, "misses")))
    return not fails

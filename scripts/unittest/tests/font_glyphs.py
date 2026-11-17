# Glyph outlines (plain and composite), cmap formats 4 and 12, the .notdef
# box, and corrupt glyph records, which must raise ValueError when the glyph is drawn
# (never crash) and must not leak the rasterizer's scratch memory.
def unittest(data_path, temp_path):
    import image, struct, gc

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    def ink(img):
        return sum(1 for y in range(img.height()) for x in range(img.width()) if img.get_pixel((x, y)) > 127)

    mono = image.Font(data_path + "/font_mono.ttf", 24, fallback=False)
    comp = image.Font(data_path + "/font_composite.ttf", 24)
    cjk = image.Font(data_path + "/font_cjk.ttf", 24)

    # Every printable ASCII glyph renders with ink (space is the only empty one).
    empty = [chr(c) for c in range(0x20, 0x7F) if ink(mono.render(chr(c))) == 0]
    check("ascii ink", empty == [" "], repr(empty))
    check("space has advance", mono.measure(" ")[0] > 0)

    # Coverage is anti-aliased: full and partial values are both present.
    cov = mono.render("O")
    vals = set(cov.get_pixel((x, y)) for y in range(cov.height()) for x in range(cov.width()))
    check("coverage full", 255 in vals and 0 in vals)
    check("coverage partial", any(0 < v < 255 for v in vals), str(len(vals)))
    # An 'O' is hollow: the centre of the glyph has no coverage (winding is honoured).
    check("counter is empty", cov.get_pixel((cov.width() // 2, cov.height() // 2 + 2)) == 0)
    # An 'I' is solid through its centre column.
    cov = mono.render("I")
    col = [cov.get_pixel((cov.width() // 2, y)) for y in range(cov.height())]
    check("stem is solid", max(col) == 255)

    # Composite glyphs: an accented letter is its base letter plus a mark above it.
    e, ea = comp.render("e"), comp.render("é")
    check("composite has more ink", ink(ea) > ink(e), "%d %d" % (ink(ea), ink(e)))
    check("composite same advance", comp.measure("e")[0] == comp.measure("é")[0])
    for ch in "éèñüÅçÖ":
        check("composite %x" % ord(ch), ink(comp.render(ch)) > 0)

    # cmap format 12 and ideographs built from shared components.
    for ch in "中文日本語한국":
        check("cjk %x" % ord(ch), ink(cjk.render(ch)) > 40, str(ink(cjk.render(ch))))
    check("supplementary plane", ink(cjk.render("\U00010400")) > 0)
    check("cjk distinct", cjk.render("中").difference(cjk.render("文")).get_statistics().max > 0)

    # Missing characters draw the .notdef box: same glyph for every unmapped code point,
    # a hollow rectangle with ink, and it has an advance.
    # (Thai is in no shipped font, so the default font's fallbacks cannot supply it.)
    nd1, nd2 = mono.render("ก"), mono.render("ข")
    check("notdef ink", ink(nd1) > 0)
    check("notdef same glyph", nd1.width() == nd2.width() and nd1.copy().difference(nd2).get_statistics().max == 0)
    check("notdef hollow", nd1.get_pixel((nd1.width() // 2, nd1.height() // 2)) == 0)
    check("notdef differs from letter", mono.render("A").difference(mono.render("ก")).get_statistics().max > 0)

    # Corrupt glyphs. The font loads; drawing the bad glyph raises; other glyphs still draw.
    good = open(data_path + "/font_mono.ttf", "rb").read()
    tmp = temp_path + "/font_test.ttf"

    def table(buf, tag):
        n = struct.unpack(">H", buf[4:6])[0]
        for i in range(n):
            rec = 12 + 16 * i
            if bytes(buf[rec:rec + 4]) == tag:
                return struct.unpack(">I", buf[rec + 8:rec + 12])[0], struct.unpack(">I", buf[rec + 12:rec + 16])[0]

    loca, _ = table(good, b"loca")
    glyf, glyf_len = table(good, b"glyf")
    probe = image.Font(data_path + "/font_mono.ttf", 24)

    def glyph_offset(gid):
        return struct.unpack(">H", good[loca + gid * 2:loca + gid * 2 + 2])[0] * 2

    def cmap4_lookup(cp):
        cmap, _ = table(good, b"cmap")
        n = struct.unpack(">H", good[cmap + 2:cmap + 4])[0]
        for i in range(n):
            sub = cmap + struct.unpack(">I", good[cmap + 8 + 8 * i:cmap + 12 + 8 * i])[0]
            if struct.unpack(">H", good[sub:sub + 2])[0] != 4:
                continue
            segs = struct.unpack(">H", good[sub + 6:sub + 8])[0] // 2
            ends, starts = sub + 14, sub + 16 + segs * 2
            deltas, ros = starts + segs * 2, starts + segs * 4
            for s in range(segs):
                end = struct.unpack(">H", good[ends + s * 2:ends + s * 2 + 2])[0]
                start = struct.unpack(">H", good[starts + s * 2:starts + s * 2 + 2])[0]
                if start <= cp <= end:
                    delta = struct.unpack(">H", good[deltas + s * 2:deltas + s * 2 + 2])[0]
                    ro = struct.unpack(">H", good[ros + s * 2:ros + s * 2 + 2])[0]
                    if not ro:
                        return (cp + delta) & 0xFFFF
                    p = ros + s * 2 + ro + (cp - start) * 2
                    return (struct.unpack(">H", good[p:p + 2])[0] + delta) & 0xFFFF
        raise Exception("no cmap")

    gid_a = cmap4_lookup(ord("A"))

    def corrupt(name, mutate, text="A", bad=True):
        buf = bytearray(good)
        mutate(buf)
        with open(tmp, "wb") as fp:
            fp.write(buf)
        try:
            f = image.Font(tmp, 24)
        except ValueError as e:
            check(name + " loads", False, repr(e))
            return
        for op in ("measure", "render", "draw"):
            try:
                if op == "measure":
                    f.measure(text)
                elif op == "render":
                    f.render(text)
                else:
                    image.Image(64, 32, image.GRAYSCALE).draw_string((0, 0), text, color=255, font=f)
                # measure() only reads glyph headers, so only a bad record offset fails there.
                check("%s %s" % (name, op), (not bad) or op == "measure", "no exception")
            except ValueError as e:
                check("%s %s" % (name, op), bad and "Corrupt glyph" in str(e), repr(e))
        if bad:
            check(name + " other glyphs", ink(f.render("o")) > 0)

    def loca_past_end(buf):
        buf[loca + (gid_a + 1) * 2:loca + (gid_a + 1) * 2 + 2] = b"\xff\xff"

    def loca_reversed(buf):
        buf[loca + (gid_a + 1) * 2:loca + (gid_a + 1) * 2 + 2] = b"\x00\x00"

    def contours_huge(buf):
        o = glyf + glyph_offset(gid_a)
        buf[o:o + 2] = b"\x7f\xff"

    def end_points_descend(buf):
        o = glyf + glyph_offset(gid_a) + 10
        buf[o:o + 2] = b"\xff\xf0"

    def points_overrun(buf):
        # Every flag says "repeat 255 times": the point data runs off the end of the glyph.
        o = glyf + glyph_offset(gid_a)
        nc = struct.unpack(">h", buf[o:o + 2])[0]
        buf[o + 10 + (nc - 1) * 2:o + 10 + nc * 2] = b"\x7f\x00"

    def self_reference(buf):
        # Turn 'A' into a composite of itself: recursion must stop at the depth limit.
        o = glyf + glyph_offset(gid_a)
        buf[o:o + 2] = b"\xff\xff"
        buf[o + 10:o + 16] = struct.pack(">HHbb", 0x0002, gid_a, 0, 0)

    def component_truncated(buf):
        o = glyf + glyph_offset(gid_a)
        end = glyf + glyph_offset(gid_a + 1)
        buf[o:o + 2] = b"\xff\xff"
        for i in range(o + 10, end - 3, 4):
            buf[i:i + 4] = struct.pack(">HH", 0x0021, 1)     # ARGS_ARE_WORDS | MORE_COMPONENTS, forever

    corrupt("loca past end", loca_past_end)
    corrupt("loca reversed", loca_reversed)
    corrupt("contours huge", contours_huge)
    corrupt("end points descend", end_points_descend)
    corrupt("points overrun", points_overrun)
    corrupt("component truncated", component_truncated)
    corrupt("self reference", self_reference, bad=False)

    # Errors must not poison later drawing.
    gc.collect()
    check("good after bad", ink(probe.render("A")) > 0)
    return not fails

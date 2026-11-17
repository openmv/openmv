# image.Font loading: ROMFS and filesystem fonts, attributes, argument checking, and
# every error the parser reports for unsupported or corrupt files. Corrupt fonts are
# built here from a good one, so each case differs from a valid font by one field.
def unittest(data_path, temp_path):
    import image, struct, gc

    fails = []

    def check(name, ok, info=""):
        if not ok:
            fails.append(name)
            print("FAIL", name, info)

    src = data_path + "/font_mono.ttf"
    good = open(src, "rb").read()
    tmp = temp_path + "/font_test.ttf"

    def table(buf, tag):
        n = struct.unpack(">H", buf[4:6])[0]
        for i in range(n):
            rec = 12 + 16 * i
            if bytes(buf[rec:rec + 4]) == tag:
                return rec, struct.unpack(">I", buf[rec + 8:rec + 12])[0], struct.unpack(">I", buf[rec + 12:rec + 16])[0]
        raise Exception("no table")

    def load(buf, size=16):
        with open(tmp, "wb") as f:
            f.write(buf)
        return image.Font(tmp, size)

    def raises(name, buf, text, exc=ValueError):
        try:
            load(buf)
            check(name, False, "no exception")
        except exc as e:
            check(name, text in str(e), repr(e))
        except Exception as e:
            check(name, False, repr(e))

    # Good fonts.
    f = image.Font(src, 16)
    check("path", f.path == src)
    check("size", f.size == 16)
    check("glyphs", f.glyphs == 96, str(f.glyphs))
    check("metrics", f.ascender > 0 and f.descender < 0 and f.line_height >= f.ascender - f.descender)
    check("fallback none", f.fallback is None)
    check("fallback false", image.Font(src, 16, fallback=False).fallback is False)
    check("bytes", f.bytes == len(good), "%d %d" % (f.bytes, len(good)))
    r = repr(f)
    check("repr has every attribute", "font_mono.ttf" in r and all(('"%s"' % k) in r for k in
          ("path", "size", "fallback", "glyphs", "bytes", "ascender", "descender", "line_height")), r)
    check("repr values", ('"glyphs":96' in r) and ('"bytes":%d' % len(good) in r) and ('"line_height":%d' % f.line_height in r), r)
    check("default font bytes", image.Font().bytes > 0)
    check("float size", image.Font(src, 13.5).size == 13.5)
    check("default size", image.Font(src).size == 10)
    d = image.Font()
    check("default font", d.size == 10 and d.glyphs > 0)
    # Its path is the ROMFS file it came from, which loads as the same font (a font built
    # into the firmware has no file, and no path).
    if d.path is not None:
        check("default font path", d.path.startswith("/rom/") and d.path.endswith(".ttf"), repr(d.path))
        again = image.Font(d.path, 10)
        check("default font path loads", again.path == d.path and again.glyphs == d.glyphs and again.bytes == d.bytes and
              again.measure("Hello") == d.measure("Hello"))
        check("default font path in repr", d.path in repr(d))
    check("default font sized", image.Font(None, 20).size == 20)
    check("line height scales", image.Font(src, 32).line_height > f.line_height)

    # The same font from the filesystem measures and renders identically.
    c = load(good)
    check("fs glyphs", c.glyphs == f.glyphs)
    check("fs measure", c.measure("Hello, World!") == f.measure("Hello, World!"))
    a, b = c.render("Hello"), f.render("Hello")
    check("fs render", a.width() == b.width() and a.difference(b).get_statistics().max == 0)
    # The copy lives on the GC heap, referenced only by the font: it must survive
    # collections while other allocations churn the heap around it.
    ref = c.render("Keep me alive")
    for i in range(20):
        junk = [bytearray(1500) for _ in range(16)]
        del junk
        gc.collect()
    check("fs font survives gc", c.render("Keep me alive").difference(ref).get_statistics().max == 0)
    del c, a, b, ref
    gc.collect()

    # Arguments.
    for name, args, exc in (("size 0", (src, 0), ValueError), ("size negative", (src, -4), ValueError),
                            ("size type", (src, "big"), TypeError), ("path type", (123,), (TypeError, OSError)),
                            ("missing file", (temp_path + "/no_such_font.ttf",), OSError)):
        try:
            image.Font(*args)
            check(name, False, "no exception")
        except exc:
            pass
        except Exception as e:
            check(name, False, repr(e))
    try:
        image.Font(src, 16, fallback="not a font")
        check("fallback type", False)
    except TypeError:
        pass

    # Not a TrueType font.
    raises("empty file", b"", "Not a TrueType")
    raises("short file", good[:11], "Not a TrueType")
    raises("bad magic", b"\xde\xad\xbe\xef" + good[4:], "Not a TrueType")
    raises("zero file", bytes(len(good)), "Not a TrueType")
    raises("collection", b"ttcf" + good[4:], "collections")
    raises("cff", b"OTTO" + good[4:], "CFF")
    check("apple magic", load(b"true" + good[4:]).glyphs == 96)

    # Truncation: every table that falls outside the file is caught.
    raises("header only", good[:12], "required table")
    for cut in (64, len(good) // 4, len(good) // 2, len(good) - 1):
        try:
            load(good[:cut])
            check("truncated %d" % cut, False, "no exception")
        except ValueError:
            pass

    # Each required table missing.
    for tag in (b"head", b"maxp", b"hhea", b"hmtx", b"loca", b"glyf", b"cmap"):
        buf = bytearray(good)
        rec, off, ln = table(buf, tag)
        buf[rec:rec + 4] = b"XXXX"
        raises("missing " + tag.decode(), buf, "required table")

    # A table that points past the end of the file.
    buf = bytearray(good)
    rec, off, ln = table(buf, b"glyf")
    buf[rec + 12:rec + 16] = struct.pack(">I", len(good))
    raises("table length out of bounds", buf, "out of bounds")
    buf = bytearray(good)
    buf[rec + 8:rec + 12] = struct.pack(">I", len(good) + 4)
    raises("table offset out of bounds", buf, "out of bounds")

    # Header fields.
    buf = bytearray(good)
    rec, off, ln = table(buf, b"head")
    buf[off + 18:off + 20] = b"\x00\x00"
    raises("units per em 0", buf, "units")
    buf = bytearray(good)
    rec, off, ln = table(buf, b"maxp")
    buf[off + 4:off + 6] = b"\x00\x00"
    raises("no glyphs", buf, "no glyphs")
    buf = bytearray(good)
    rec, off, ln = table(buf, b"hhea")
    buf[off + 34:off + 36] = b"\x00\x00"
    raises("no hmetrics", buf, "metrics")
    buf = bytearray(good)
    buf[off + 34:off + 36] = b"\xff\xff"
    raises("too many hmetrics", buf, "metrics")
    buf = bytearray(good)
    rec, off, ln = table(buf, b"maxp")
    buf[off + 4:off + 6] = b"\xff\xff"          # more glyphs than loca has entries
    raises("loca too short", buf, "required table")

    # cmap: no subtable in a supported format, and a segment count that overruns the file.
    buf = bytearray(good)
    rec, off, ln = table(buf, b"cmap")
    n = struct.unpack(">H", buf[off + 2:off + 4])[0]
    for i in range(n):
        sub = off + struct.unpack(">I", buf[off + 8 + 8 * i:off + 12 + 8 * i])[0]
        buf[sub:sub + 2] = b"\x00\x06"
    raises("cmap format unsupported", buf, "cmap")
    buf = bytearray(good)
    for i in range(n):
        sub = off + struct.unpack(">I", buf[off + 8 + 8 * i:off + 12 + 8 * i])[0]
        buf[sub + 6:sub + 8] = b"\xff\xfe"
    raises("cmap segments overrun", buf, "cmap")
    buf = bytearray(good)
    buf[off + 2:off + 4] = b"\x00\x00"
    raises("cmap no subtables", buf, "cmap")

    # A font object that failed to load leaves nothing behind and good fonts still work.
    gc.collect()
    check("good after bad", image.Font(src, 16).measure("ok") == f.measure("ok"))
    return not fails

#!/usr/bin/env python3
#
# This file is part of the OpenMV project.
# Copyright (C) 2026 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Tests for the TrueType subsetter (tools/mkfont.py). These run on the build host, not on
# a camera: CI runs every test_*.py in this directory with the firmware unit tests
# (ci_run_host_tests in tools/ci.sh). Standard library only; to run them by hand:
#
#     python3 -m unittest discover -s scripts/unittest/host

import os
import struct
import subprocess
import sys
import tempfile
import unittest

TOP = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..")
TOOLS = os.path.join(TOP, "tools")
sys.path.insert(0, TOOLS)
import mkfont  # noqa: E402

FONTS = os.path.join(TOP, "lib", "fonts")
ASCII = sorted(mkfont.parse_unicodes("U+0020-007E"))
KEPT_TABLES = [b"cmap", b"glyf", b"head", b"hhea", b"hmtx", b"loca", b"maxp"]


def load(name):
    with open(os.path.join(FONTS, name), "rb") as f:
        return f.read()


def outline(font, gid, depth=0):
    """A glyph as comparable data: its bytes without instructions, components resolved recursively."""
    # (Glyph records are padded with zeros to the loca alignment, which differs between files.)
    gl = mkfont.strip_instructions(font.glyph(gid)).rstrip(b"\0")
    if len(gl) < 10 or mkfont.s16(gl, 0) >= 0:
        return gl
    assert depth < 8
    parts = []
    last = 10
    for off, cg in mkfont.components(gl):
        parts.append(gl[last:off])
        parts.append(outline(font, cg, depth + 1))
        last = off + 2
    parts.append(gl[last:])
    return (gl[:10], tuple(parts))


class ParseUnicodes(unittest.TestCase):
    def test_forms(self):
        self.assertEqual(mkfont.parse_unicodes("U+0041"), {0x41})
        self.assertEqual(mkfont.parse_unicodes("U+0041-0043"), {0x41, 0x42, 0x43})
        self.assertEqual(mkfont.parse_unicodes("U+41,U+0061-0062"), {0x41, 0x61, 0x62})
        self.assertEqual(mkfont.parse_unicodes(" U+0041 , U+0042 ,"), {0x41, 0x42})
        self.assertEqual(mkfont.parse_unicodes("u+1f600"), {0x1F600})
        self.assertEqual(mkfont.parse_unicodes("U+0041-U+0042"), {0x41, 0x42})
        self.assertEqual(mkfont.parse_unicodes(""), set())

    def test_duplicates(self):
        self.assertEqual(mkfont.parse_unicodes("U+0042,U+0041,U+0041-0042"), {0x41, 0x42})

    def test_bad(self):
        for spec in ("U+ZZZZ", "0041-", "U+0043-0041", "U+0041-0043x", "U+110000", "-0041", "hello"):
            with self.assertRaises(ValueError):
                mkfont.parse_unicodes(spec)


class Subset(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.mono = load("RobotoMono-Regular.ttf")
        cls.sans = load("Roboto-Regular.ttf")
        cls.cjk = load("DroidSansFallback.ttf")
        cls.naskh = load("DroidNaskh-Regular.ttf")

    def check_same_glyphs(self, data, cps):
        src = mkfont.Font(data)
        out, nglyphs, nmapped = mkfont.subset(data, cps)
        dst = mkfont.Font(out)
        present = [c for c in cps if c in src.cmap]
        self.assertEqual(nmapped, len(present))
        self.assertEqual(sorted(dst.cmap), sorted(present))
        self.assertEqual(dst.num_glyphs, nglyphs)
        self.assertEqual(dst.units_per_em, src.units_per_em)
        for c in present:
            sg, dg = src.cmap[c], dst.cmap[c]
            self.assertEqual(src.hmetrics[sg], dst.hmetrics[dg], "metrics U+%04X" % c)
            self.assertEqual(outline(src, sg), outline(dst, dg), "outline U+%04X" % c)
        return src, dst, out

    def test_ascii(self):
        src, dst, out = self.check_same_glyphs(self.sans, ASCII)
        self.assertEqual(len(dst.cmap), 95)
        self.assertLess(len(out), 12000)

    def test_extended_with_composites(self):
        cps = sorted(mkfont.parse_unicodes("U+0020-007E,U+00A0-024F,U+0370-03FF,U+0400-04FF"))
        src, dst, out = self.check_same_glyphs(self.sans, cps)
        # Accented letters are composites; their components must be pulled in and renumbered.
        gl = dst.glyph(dst.cmap[0xE9])
        self.assertLess(mkfont.s16(gl, 0), 0)
        for _, cg in mkfont.components(gl):
            self.assertLess(cg, dst.num_glyphs)

    def test_component_not_requested(self):
        # Only the composite is requested: the base letter and the accent still come along.
        out, nglyphs, nmapped = mkfont.subset(self.sans, [0xE9])
        self.assertEqual(nmapped, 1)
        self.assertGreaterEqual(nglyphs, 3)     # .notdef + composite + at least one component

    def test_cjk_and_supplementary_plane(self):
        cps = [ord(c) for c in "中文日本語한국"] + [0x10400]
        src, dst, out = self.check_same_glyphs(self.cjk, cps)
        self.assertIn(0x10400, dst.cmap)

    def test_arabic(self):
        self.check_same_glyphs(self.naskh, sorted(mkfont.parse_unicodes("U+0621-064A,U+FE70-FEFC")))

    def test_only_needed_tables(self):
        out, _, _ = mkfont.subset(self.sans, ASCII)
        n = mkfont.u16(out, 4)
        tags = [out[12 + 16 * i:16 + 16 * i] for i in range(n)]
        self.assertEqual(tags, KEPT_TABLES)

    def test_table_directory_and_checksums(self):
        out, _, _ = mkfont.subset(self.sans, ASCII)
        self.assertEqual(mkfont.u32(out, 0), 0x00010000)
        n = mkfont.u16(out, 4)
        for i in range(n):
            tag, csum, off, length = struct.unpack(">4sIII", out[12 + 16 * i:28 + 16 * i])
            self.assertEqual(off % 4, 0, tag)
            self.assertLessEqual(off + length, len(out), tag)
            table = bytearray(out[off:off + length])
            if tag == b"head":
                table[8:12] = b"\0\0\0\0"
            self.assertEqual(mkfont.checksum(bytes(table)), csum, tag)
        # The whole file sums to the magic number once head.checkSumAdjustment is set.
        self.assertEqual(mkfont.checksum(out), 0xB1B0AFBA)

    def test_hinting_removed(self):
        src = mkfont.Font(self.cjk)
        out, _, _ = mkfont.subset(self.cjk, [0x4E2D])
        dst = mkfont.Font(out)
        gl = dst.glyph(dst.cmap[0x4E2D])
        if mkfont.s16(gl, 0) >= 0:
            nc = mkfont.s16(gl, 0)
            self.assertEqual(mkfont.u16(gl, 10 + nc * 2), 0)        # instructionLength
        self.assertNotIn(b"fpgm", out)
        self.assertNotIn(b"prep", out)

    def test_loca_format(self):
        small, _, _ = mkfont.subset(self.sans, ASCII)
        self.assertEqual(mkfont.Font(small).index_to_loc_format, 0)
        hanzi = [c for c in sorted(mkfont.Font(self.cjk).cmap) if 0x4E00 <= c <= 0x9FFF][:1500]
        big, _, _ = mkfont.subset(self.cjk, hanzi)
        f = mkfont.Font(big)
        self.assertGreaterEqual(len(f.tables[b"glyf"]), 0x20000)
        self.assertEqual(f.index_to_loc_format, 1)
        self.assertEqual(outline(mkfont.Font(self.cjk), mkfont.Font(self.cjk).cmap[hanzi[-1]]), outline(f, f.cmap[hanzi[-1]]))

    def test_maxp_matches_glyphs(self):
        out, nglyphs, _ = mkfont.subset(self.sans, ASCII)
        f = mkfont.Font(out)
        maxp = f.tables[b"maxp"]
        self.assertEqual(mkfont.u16(maxp, 4), nglyphs)
        max_points = max_contours = 0
        for g in range(f.num_glyphs):
            gl = f.glyph(g)
            if len(gl) >= 10 and mkfont.s16(gl, 0) > 0:
                nc = mkfont.s16(gl, 0)
                max_contours = max(max_contours, nc)
                max_points = max(max_points, mkfont.u16(gl, 10 + (nc - 1) * 2) + 1)
        self.assertEqual((mkfont.u16(maxp, 6), mkfont.u16(maxp, 8)), (max_points, max_contours))
        self.assertEqual(mkfont.u16(f.tables[b"hhea"], 34), nglyphs)
        self.assertEqual(len(f.tables[b"hmtx"]), 4 * nglyphs)

    def test_notdef_kept_or_synthesized(self):
        # Roboto has a .notdef outline: it is kept as is.
        src = mkfont.Font(self.sans)
        out, _, _ = mkfont.subset(self.sans, ASCII)
        self.assertEqual(outline(src, 0), outline(mkfont.Font(out), 0))
        # Roboto Mono's is empty: a hollow box (two contours, eight points) is made for it.
        self.assertEqual(len(mkfont.Font(self.mono).glyph(0)), 0)
        out, _, _ = mkfont.subset(self.mono, ASCII)
        f = mkfont.Font(out)
        gl = f.glyph(0)
        self.assertEqual(mkfont.s16(gl, 0), 2)
        self.assertEqual(mkfont.u16(gl, 12) + 1, 8)
        x0, y0, x1, y1 = struct.unpack(">hhhh", gl[2:10])
        self.assertTrue(0 <= x0 < x1 <= f.hmetrics[0][0])
        self.assertTrue(0 <= y0 < y1 <= f.units_per_em)

    def test_line_metrics_override(self):
        src = mkfont.Font(self.mono)
        out, _, _ = mkfont.subset(self.mono, ASCII, ascender=0.8, descender=0.2)
        hhea = mkfont.Font(out).tables[b"hhea"]
        upem = src.units_per_em
        self.assertEqual((mkfont.s16(hhea, 4), mkfont.s16(hhea, 6), mkfont.s16(hhea, 8)),
                         (round(0.8 * upem), -round(0.2 * upem), 0))
        # Without the arguments the font's own metrics are kept.
        plain, _, _ = mkfont.subset(self.mono, ASCII)
        self.assertEqual(mkfont.Font(plain).tables[b"hhea"][4:10], src.tables[b"hhea"][4:10])
        # Glyphs are untouched either way.
        self.assertEqual(mkfont.Font(plain).tables[b"glyf"], mkfont.Font(out).tables[b"glyf"])
        for bad in ((0.8, None), (None, 0.2), (0, 0.2), (0.8, -0.1)):
            with self.assertRaises(ValueError):
                mkfont.subset(self.mono, ASCII, ascender=bad[0], descender=bad[1])

    def test_missing_code_points_are_skipped(self):
        out, nglyphs, nmapped = mkfont.subset(self.mono, [0x41, 0x4E2D, 0x1F600])
        self.assertEqual(nmapped, 1)
        self.assertEqual(sorted(mkfont.Font(out).cmap), [0x41])

    def test_nothing_mapped_still_valid(self):
        out, nglyphs, nmapped = mkfont.subset(self.mono, [0x4E2D])
        self.assertEqual((nglyphs, nmapped), (1, 0))

    def test_idempotent(self):
        once, _, _ = mkfont.subset(self.sans, ASCII)
        twice, _, _ = mkfont.subset(once, ASCII)
        self.assertEqual(once, twice)

    def test_deterministic_and_order_independent(self):
        a, _, _ = mkfont.subset(self.sans, ASCII)
        b, _, _ = mkfont.subset(self.sans, list(reversed(ASCII)) + ASCII)
        self.assertEqual(a, b)


class Cmap(unittest.TestCase):
    def roundtrip(self, mapping):
        table = mkfont.build_cmap(mapping)
        self.assertEqual(mkfont.Font.parse_cmap(table), mapping)
        return table

    def test_contiguous(self):
        self.roundtrip({c: c - 0x1F for c in range(0x20, 0x7F)})

    def test_scattered(self):
        self.roundtrip({0x41: 5, 0x43: 1, 0x100: 9, 0x2713: 2, 0xFFFD: 3})

    def test_delta_and_explicit_segments(self):
        # A run whose glyph ids are not consecutive cannot use a delta segment.
        self.roundtrip({0x41: 10, 0x42: 3, 0x43: 7, 0x44: 8, 0x45: 9, 0x60: 1})

    def test_end_of_bmp(self):
        # (U+FFFF itself is the format 4 terminator and a noncharacter; it cannot be mapped.)
        self.roundtrip({0xFFFD: 1, 0xFFFE: 3, 0x41: 2})

    def test_empty(self):
        self.roundtrip({})

    def test_format_12_only_when_needed(self):
        bmp = mkfont.build_cmap({0x41: 1})
        self.assertEqual(mkfont.u16(bmp, 2), 1)                    # one subtable
        smp = self.roundtrip({0x41: 1, 0x1F600: 2, 0x1F601: 3, 0x20000: 4})
        self.assertEqual(mkfont.u16(smp, 2), 2)                    # (3,1) format 4 and (3,10) format 12

    def test_rejects_unsupported_subtables(self):
        table = bytearray(mkfont.build_cmap({0x41: 1}))
        sub = mkfont.u32(table, 8)
        table[sub:sub + 2] = b"\0\6"
        with self.assertRaises(ValueError):
            mkfont.Font.parse_cmap(bytes(table))


class Rejects(unittest.TestCase):
    def test_not_truetype(self):
        good = load("Roboto-Regular.ttf")
        for data in (b"", b"\0" * 64, b"OTTO" + good[4:], b"ttcf" + good[4:], b"wOFF" + good[4:]):
            with self.assertRaises(ValueError):
                mkfont.subset(data, ASCII)

    def test_missing_table(self):
        good = bytearray(load("Roboto-Regular.ttf"))
        n = mkfont.u16(good, 4)
        for i in range(n):
            if bytes(good[12 + 16 * i:16 + 16 * i]) == b"glyf":
                good[12 + 16 * i:16 + 16 * i] = b"XXXX"
        with self.assertRaises(ValueError):
            mkfont.subset(bytes(good), ASCII)

    def test_truncated(self):
        good = load("Roboto-Regular.ttf")
        with self.assertRaises(ValueError):
            mkfont.subset(good[:len(good) // 2], ASCII)


class CommandLine(unittest.TestCase):
    def run_tool(self, *args):
        return subprocess.run([sys.executable, os.path.join(TOOLS, "mkfont.py")] + list(args),
                              capture_output=True, text=True)

    def test_unicodes_and_text(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "out.ttf")
            r = self.run_tool(os.path.join(FONTS, "Roboto-Regular.ttf"), out, "--unicodes", "U+0041-0043", "--text", "é€")
            self.assertEqual(r.returncode, 0, r.stderr)
            with open(out, "rb") as f:
                self.assertEqual(sorted(mkfont.Font(f.read()).cmap), [0x41, 0x42, 0x43, 0xE9, 0x20AC])

    def test_c_array(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "font.c")
            r = self.run_tool(os.path.join(FONTS, "RobotoMono-Regular.ttf"), out, "--unicodes", "U+0041", "--c-array", "omv_font_builtin")
            self.assertEqual(r.returncode, 0, r.stderr)
            src = open(out).read()
            self.assertIn("omv_font_builtin[]", src)
            self.assertIn("omv_font_builtin_len", src)
            data, _, _ = mkfont.subset(load("RobotoMono-Regular.ttf"), [0x41])
            self.assertEqual(src.count("0x"), len(data))

    def test_line_metrics(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "out.ttf")
            r = self.run_tool(os.path.join(FONTS, "RobotoMono-Regular.ttf"), out, "--unicodes", "U+0041",
                              "--ascender", "0.8", "--descender", "0.2")
            self.assertEqual(r.returncode, 0, r.stderr)
            with open(out, "rb") as f:
                font = mkfont.Font(f.read())
            self.assertEqual(mkfont.s16(font.tables[b"hhea"], 4), round(0.8 * font.units_per_em))

    def test_nothing_to_keep(self):
        with tempfile.TemporaryDirectory() as d:
            r = self.run_tool(os.path.join(FONTS, "Roboto-Regular.ttf"), os.path.join(d, "out.ttf"))
            self.assertNotEqual(r.returncode, 0)

    def test_subset_file(self):
        with tempfile.TemporaryDirectory() as d:
            out = os.path.join(d, "out.ttf")
            mkfont.subset_file(os.path.join(FONTS, "Roboto-Regular.ttf"), out, "U+0041", "B")
            with open(out, "rb") as f:
                self.assertEqual(sorted(mkfont.Font(f.read()).cmap), [0x41, 0x42])


if __name__ == "__main__":
    unittest.main()

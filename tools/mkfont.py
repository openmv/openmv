#!/usr/bin/env python3
#
# This file is part of the OpenMV project.
# Copyright (C) 2026 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# Subset a TrueType (glyf) font to a set of code points using only the Python
# standard library. Keeps cmap, glyf, head, hhea, hmtx, loca and maxp, which is
# everything the firmware's rasterizer reads. Hinting, kerning, names and every
# other optional table are dropped.

import argparse
import struct
import sys

KEEP_TABLES = (b"cmap", b"glyf", b"head", b"hhea", b"hmtx", b"loca", b"maxp")

# Composite glyph flags.
ARG_1_AND_2_ARE_WORDS = 0x0001
WE_HAVE_A_SCALE = 0x0008
MORE_COMPONENTS = 0x0020
WE_HAVE_AN_X_AND_Y_SCALE = 0x0040
WE_HAVE_A_TWO_BY_TWO = 0x0080
WE_HAVE_INSTRUCTIONS = 0x0100


def u16(b, o):
    return struct.unpack_from(">H", b, o)[0]


def s16(b, o):
    return struct.unpack_from(">h", b, o)[0]


def u32(b, o):
    return struct.unpack_from(">I", b, o)[0]


def checksum(data):
    data = data + b"\0" * (-len(data) % 4)
    return sum(struct.unpack(">%dI" % (len(data) // 4), data)) & 0xFFFFFFFF


class Font:
    def __init__(self, data):
        self.data = data
        tag = data[:4]
        if tag == b"ttcf":
            raise ValueError("TrueType collections are not supported")
        if tag == b"OTTO":
            raise ValueError("CFF/OpenType fonts are not supported (need glyf outlines)")
        if tag not in (b"\0\1\0\0", b"true"):
            raise ValueError("not a TrueType font")
        num_tables = u16(data, 4)
        self.tables = {}
        for i in range(num_tables):
            rec = 12 + i * 16
            t = data[rec:rec + 4]
            off, length = u32(data, rec + 8), u32(data, rec + 12)
            if off + length > len(data):
                raise ValueError("table %r out of bounds" % t)
            self.tables[t] = data[off:off + length]
        for t in KEEP_TABLES:
            if t not in self.tables:
                raise ValueError("missing required table %r" % t)

        head = self.tables[b"head"]
        self.units_per_em = u16(head, 18)
        self.index_to_loc_format = s16(head, 50)
        self.num_glyphs = u16(self.tables[b"maxp"], 4)
        self.num_hmetrics = u16(self.tables[b"hhea"], 34)

        loca = self.tables[b"loca"]
        n = self.num_glyphs + 1
        if self.index_to_loc_format == 0:
            self.loca = [2 * v for v in struct.unpack_from(">%dH" % n, loca)]
        else:
            self.loca = list(struct.unpack_from(">%dI" % n, loca))

        hmtx = self.tables[b"hmtx"]
        self.hmetrics = []
        for g in range(self.num_glyphs):
            if g < self.num_hmetrics:
                aw, lsb = struct.unpack_from(">Hh", hmtx, g * 4)
            else:
                aw = self.hmetrics[self.num_hmetrics - 1][0]
                lsb = s16(hmtx, self.num_hmetrics * 4 + (g - self.num_hmetrics) * 2)
            self.hmetrics.append((aw, lsb))

        self.cmap = self.parse_cmap(self.tables[b"cmap"])

    def glyph(self, gid):
        return self.tables[b"glyf"][self.loca[gid]:self.loca[gid + 1]]

    @staticmethod
    def parse_cmap(cmap):
        # Prefer a full Unicode subtable (3,10) or (0,4), then BMP (3,1) or (0,x).
        best, best_rank = None, -1
        for i in range(u16(cmap, 2)):
            pid, eid, off = struct.unpack_from(">HHI", cmap, 4 + i * 8)
            fmt = u16(cmap, off)
            if fmt not in (4, 12):
                continue
            rank = {(3, 10): 4, (0, 4): 3, (0, 6): 3, (3, 1): 2, (0, 3): 1}.get((pid, eid), 0)
            if rank > best_rank:
                best, best_rank = off, rank
        if best is None:
            raise ValueError("no usable Unicode cmap subtable (format 4 or 12)")
        mapping = {}
        fmt = u16(cmap, best)
        if fmt == 4:
            seg_x2 = u16(cmap, best + 6)
            segs = seg_x2 // 2
            ends = struct.unpack_from(">%dH" % segs, cmap, best + 14)
            starts = struct.unpack_from(">%dH" % segs, cmap, best + 16 + seg_x2)
            deltas = struct.unpack_from(">%dh" % segs, cmap, best + 16 + 2 * seg_x2)
            ro_base = best + 16 + 3 * seg_x2
            ros = struct.unpack_from(">%dH" % segs, cmap, ro_base)
            for i in range(segs):
                for c in range(starts[i], min(ends[i], 0xFFFE) + 1):
                    if ros[i] == 0:
                        g = (c + deltas[i]) & 0xFFFF
                    else:
                        p = ro_base + i * 2 + ros[i] + (c - starts[i]) * 2
                        g = u16(cmap, p)
                        if g:
                            g = (g + deltas[i]) & 0xFFFF
                    if g:
                        mapping[c] = g
        else:
            ngroups = u32(cmap, best + 12)
            for i in range(ngroups):
                sc, ec, sg = struct.unpack_from(">III", cmap, best + 16 + i * 12)
                for c in range(sc, ec + 1):
                    mapping[c] = sg + (c - sc)
        return mapping


def components(glyph):
    """Yield (offset_of_glyph_index, glyph_index) for each component of a composite glyph."""
    if len(glyph) < 10 or s16(glyph, 0) >= 0:
        return
    p = 10
    while True:
        flags, gid = struct.unpack_from(">HH", glyph, p)
        yield p + 2, gid
        p += 4
        p += 4 if (flags & ARG_1_AND_2_ARE_WORDS) else 2
        if flags & WE_HAVE_A_SCALE:
            p += 2
        elif flags & WE_HAVE_AN_X_AND_Y_SCALE:
            p += 4
        elif flags & WE_HAVE_A_TWO_BY_TWO:
            p += 8
        if not (flags & MORE_COMPONENTS):
            break


def strip_instructions(gl):
    """Drop TrueType hinting bytecode from a glyph (fpgm/prep/cvt are not kept)."""
    if len(gl) < 10:
        return gl
    nc = s16(gl, 0)
    if nc >= 0:
        p = 10 + nc * 2
        ilen = u16(gl, p)
        if ilen == 0:
            return gl
        return gl[:p] + b"\0\0" + gl[p + 2 + ilen:]
    # Composite: instructions follow the last component when the flag is set.
    p, end = 10, 10
    out = bytearray(gl)
    while True:
        flags = u16(gl, p)
        struct.pack_into(">H", out, p, flags & ~WE_HAVE_INSTRUCTIONS)
        p += 4
        p += 4 if (flags & ARG_1_AND_2_ARE_WORDS) else 2
        if flags & WE_HAVE_A_SCALE:
            p += 2
        elif flags & WE_HAVE_AN_X_AND_Y_SCALE:
            p += 4
        elif flags & WE_HAVE_A_TWO_BY_TWO:
            p += 8
        if not (flags & MORE_COMPONENTS):
            end = p
            break
    return bytes(out[:end])


def parse_unicodes(spec):
    cps = set()
    for part in spec.replace(" ", "").split(","):
        if not part:
            continue
        lo, dash, hi = part.partition("-")
        if dash and not hi:
            raise ValueError("bad code point range %r" % part)
        lo = int(lo.replace("U+", "").replace("u+", ""), 16)
        hi = int(hi.replace("U+", "").replace("u+", ""), 16) if hi else lo
        if (hi < lo) or (hi > 0x10FFFF):
            raise ValueError("bad code point range %r" % part)
        cps.update(range(lo, hi + 1))
    return cps


def build_cmap4(mapping):
    # Segments of consecutive code points; consecutive glyph ids collapse into idDelta.
    cps = sorted(c for c in mapping if c <= 0xFFFF)
    segs = []  # [start, end, delta or None, [gids]]
    for c in cps:
        g = mapping[c]
        if segs and segs[-1][1] == c - 1:
            s = segs[-1]
            s[1] = c
            s[3].append(g)
            if s[2] is not None and (g - c) & 0xFFFF != s[2]:
                s[2] = None
            continue
        segs.append([c, c, (g - c) & 0xFFFF, [g]])
    segs.append([0xFFFF, 0xFFFF, 1, [0]])
    n = len(segs)
    ends = [s[1] for s in segs]
    starts = [s[0] for s in segs]
    deltas, ros, gia = [], [], []
    for i, s in enumerate(segs):
        if s[2] is not None:
            deltas.append(s[2])
            ros.append(0)
        else:
            deltas.append(0)
            ros.append((n - i) * 2 + len(gia) * 2)
            gia.extend(s[3])
    sr = 1
    while sr * 2 <= n:
        sr *= 2
    es = sr.bit_length() - 1
    sr *= 2
    body = struct.pack(">%dH" % n, *ends) + b"\0\0" + struct.pack(">%dH" % n, *starts)
    body += struct.pack(">%dH" % n, *deltas) + struct.pack(">%dH" % n, *ros)
    body += struct.pack(">%dH" % len(gia), *gia)
    length = 14 + len(body)
    return struct.pack(">HHHHHHH", 4, length, 0, n * 2, sr, es, n * 2 - sr) + body


def build_cmap12(mapping):
    groups = []
    for c in sorted(mapping):
        g = mapping[c]
        if groups and groups[-1][1] == c - 1 and groups[-1][2] + (c - groups[-1][0]) == g:
            groups[-1][1] = c
        else:
            groups.append([c, c, g])
    body = b"".join(struct.pack(">III", *grp) for grp in groups)
    return struct.pack(">HHIII", 12, 0, 16 + len(body), 0, len(groups)) + body


def build_cmap(mapping):
    subs = [((3, 1), build_cmap4(mapping))]
    if any(c > 0xFFFF for c in mapping):
        subs.append(((3, 10), build_cmap12(mapping)))
    hdr = struct.pack(">HH", 0, len(subs))
    off = 4 + 8 * len(subs)
    recs, data = b"", b""
    for (pid, eid), sub in subs:
        recs += struct.pack(">HHI", pid, eid, off + len(data))
        data += sub
    return hdr + recs + data


def notdef_box(font):
    """A hollow rectangle for fonts whose .notdef glyph has no outline (e.g. Roboto Mono), so
    that characters missing from a subset show up as a box instead of blank space."""
    upem = font.units_per_em
    adv = font.hmetrics[0][0] or upem // 2
    x0, x1 = int(adv * 0.1), int(adv * 0.9)
    y0, y1 = 0, int(upem * 0.7)
    t = max(1, int(upem * 0.08))
    pts = [(x0, y0), (x0, y1), (x1, y1), (x1, y0),
           (x0 + t, y0 + t), (x1 - t, y0 + t), (x1 - t, y1 - t), (x0 + t, y1 - t)]
    gl = struct.pack(">hhhhh", 2, x0, y0, x1, y1)
    gl += struct.pack(">HHH", 3, 7, 0)
    gl += b"\x01" * len(pts)
    px = py = 0
    xs, ys = b"", b""
    for x, y in pts:
        xs += struct.pack(">h", x - px)
        ys += struct.pack(">h", y - py)
        px, py = x, y
    return gl + xs + ys


def subset(data, codepoints, ascender=None, descender=None):
    """Subset a font to a set of code points. Returns (bytes, glyphs, mapped code points).

    ascender and descender, in ems, replace the font's line metrics (with no line gap): the
    baseline of the first line sits ascender ems below the top of the text and lines are
    ascender + descender ems apart. With 0.8 and 0.2 a line of text is exactly as tall as the
    font size, which suits overlays laid out on a pixel grid better than a font's own, looser
    spacing."""
    font = Font(data)
    keep = {0}
    mapping = {}
    for c in codepoints:
        g = font.cmap.get(c)
        if g is not None and g < font.num_glyphs:
            mapping[c] = g
            keep.add(g)
    # Composite closure.
    todo = list(keep)
    while todo:
        g = todo.pop()
        for _, cg in components(font.glyph(g)):
            if cg < font.num_glyphs and cg not in keep:
                keep.add(cg)
                todo.append(cg)
    order = sorted(keep)
    new_id = {g: i for i, g in enumerate(order)}

    # glyf + loca (glyphs padded to 4 bytes so short loca offsets stay even).
    glyf, loca = bytearray(), []
    max_points = max_contours = 0
    for g in order:
        gl = bytearray(strip_instructions(font.glyph(g)))
        if g == 0 and not gl:
            gl = bytearray(notdef_box(font))
        if gl:
            nc = s16(gl, 0)
            if nc < 0:
                for off, cg in components(gl):
                    struct.pack_into(">H", gl, off, new_id[cg])
            else:
                max_contours = max(max_contours, nc)
                if nc:
                    max_points = max(max_points, u16(gl, 10 + (nc - 1) * 2) + 1)
        loca.append(len(glyf))
        glyf += gl
        glyf += b"\0" * (-len(glyf) % 4)
    loca.append(len(glyf))
    long_loca = loca[-1] >= 0x20000
    if long_loca:
        loca_t = struct.pack(">%dI" % len(loca), *loca)
    else:
        loca_t = struct.pack(">%dH" % len(loca), *(v // 2 for v in loca))

    # hmtx/hhea: full metrics for every glyph.
    hmtx = b"".join(struct.pack(">Hh", *font.hmetrics[g]) for g in order)
    hhea = bytearray(font.tables[b"hhea"])
    struct.pack_into(">H", hhea, 34, len(order))
    if (ascender is not None) or (descender is not None):
        if (ascender is None) or (descender is None) or (ascender <= 0) or (descender < 0):
            raise ValueError("ascender and descender must be given together, in ems")
        struct.pack_into(">hhh", hhea, 4, round(ascender * font.units_per_em),
                         -round(descender * font.units_per_em), 0)

    maxp = bytearray(font.tables[b"maxp"])
    struct.pack_into(">H", maxp, 4, len(order))
    if len(maxp) >= 32:
        struct.pack_into(">HH", maxp, 6, max_points, max_contours)

    head = bytearray(font.tables[b"head"])
    struct.pack_into(">I", head, 8, 0)  # checkSumAdjustment, fixed up below
    struct.pack_into(">h", head, 50, 1 if long_loca else 0)

    tables = {
        b"cmap": build_cmap({c: new_id[g] for c, g in mapping.items()}),
        b"glyf": bytes(glyf),
        b"head": bytes(head),
        b"hhea": bytes(hhea),
        b"hmtx": hmtx,
        b"loca": loca_t,
        b"maxp": bytes(maxp),
    }

    # Table directory + data, tags sorted, 4-byte aligned.
    tags = sorted(tables)
    n = len(tags)
    sr = 1
    while sr * 2 <= n:
        sr *= 2
    es = sr.bit_length() - 1
    sr *= 16
    out = bytearray(struct.pack(">IHHHH", 0x00010000, n, sr, es, n * 16 - sr))
    off = 12 + 16 * n
    body = bytearray()
    for t in tags:
        d = tables[t]
        out += struct.pack(">4sIII", t, checksum(d), off + len(body), len(d))
        body += d + b"\0" * (-len(d) % 4)
    out += body
    adj = (0xB1B0AFBA - checksum(bytes(out))) & 0xFFFFFFFF
    head_off = u32(out, 12 + 16 * tags.index(b"head") + 8)
    struct.pack_into(">I", out, head_off + 8, adj)
    return bytes(out), len(order), len(mapping)


def subset_file(input_path, output_path, unicodes="", text="", ascender=None, descender=None):
    """Subset input_path to output_path. Returns (input_size, output_size, glyphs, mapped)."""
    cps = parse_unicodes(unicodes) | {ord(c) for c in text}
    if not cps:
        raise ValueError("nothing to keep: pass unicodes and/or text")
    with open(input_path, "rb") as f:
        data = f.read()
    out, nglyphs, nmapped = subset(data, cps, ascender, descender)
    with open(output_path, "wb") as f:
        f.write(out)
    return len(data), len(out), nglyphs, nmapped


def c_array(data, name):
    """Render a font as a C source file (for boards without a ROMFS)."""
    lines = ["// Generated by tools/mkfont.py. Do not edit.", "#include <stdint.h>",
             "const uint32_t %s_len = %d;" % (name, len(data)),
             "const uint8_t %s[] __attribute__((aligned(4))) = {" % name]
    for i in range(0, len(data), 16):
        lines.append("    " + "".join("0x%02x, " % b for b in data[i:i + 16]).rstrip())
    lines.append("};")
    return "\n".join(lines) + "\n"


def main():
    p = argparse.ArgumentParser(description="Subset a TrueType font (stdlib only).")
    p.add_argument("input")
    p.add_argument("output")
    p.add_argument("--unicodes", default="", help="e.g. U+0020-007E,U+00A0-00FF")
    p.add_argument("--text", default="", help="literal characters to keep")
    p.add_argument("--ascender", type=float, help="line ascent in ems (with --descender), e.g. 0.8")
    p.add_argument("--descender", type=float, help="line descent in ems (with --ascender), e.g. 0.2")
    p.add_argument("--c-array", metavar="NAME", help="write a C source file defining NAME[] and NAME_len")
    args = p.parse_args()
    cps = parse_unicodes(args.unicodes) | {ord(c) for c in args.text}
    if not cps:
        p.error("nothing to keep: pass --unicodes and/or --text")
    with open(args.input, "rb") as f:
        data = f.read()
    out, nglyphs, nmapped = subset(data, cps, args.ascender, args.descender)
    if args.c_array:
        with open(args.output, "w") as f:
            f.write(c_array(out, args.c_array))
    else:
        with open(args.output, "wb") as f:
            f.write(out)
    print("%s: %d -> %d bytes, %d glyphs (%d code points mapped, %d requested)"
          % (args.output, len(data), len(out), nglyphs, nmapped, len(cps)))


if __name__ == "__main__":
    main()

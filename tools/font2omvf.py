#!/usr/bin/env python3
#
# SPDX-License-Identifier: MIT
#
# Copyright (C) 2026 OpenMV, LLC.
#
# Convert a TrueType/OpenType font into an OpenMV bitmap font file.

import argparse
import struct

from PIL import Image, ImageDraw, ImageFont


MAGIC = b"OMVF"
VERSION = 1
HEADER_SIZE = 24
GLYPH_SIZE = 8


def pack_bitmap(image, width, height):
    row_stride = (width + 7) // 8
    data = bytearray()

    for y in range(height):
        for byte_col in range(row_stride):
            value = 0
            for bit in range(8):
                x = (byte_col * 8) + bit
                if x < width and image.getpixel((x, y)):
                    value |= 1 << (7 - bit)
            data.append(value)

    return bytes(data), row_stride


def render_glyph(font, ch, width, height, threshold):
    image = Image.new("L", (width, height), 0)
    draw = ImageDraw.Draw(image)
    bbox = font.getbbox(ch)

    if bbox is not None:
        ascent, descent = font.getmetrics()
        glyph_w = bbox[2] - bbox[0]
        x = ((width - glyph_w) // 2) - bbox[0]
        y = (height - (ascent + descent)) // 2
        draw.text((x, y), ch, fill=255, font=font)

    return image.point(lambda p: 255 if p >= threshold else 0)


def convert(args):
    font = ImageFont.truetype(args.font, args.size)
    if len(args.first) != 1 or len(args.last) != 1:
        raise ValueError("--first and --last must be single characters")

    first = ord(args.first)
    last = ord(args.last)

    if last > 127:
        raise ValueError("only ASCII characters are supported")
    if first > last:
        raise ValueError("--first must be less than or equal to --last")
    if first > ord(" ") or last < ord(" "):
        raise ValueError("font range must include the space character")

    chars = [chr(i) for i in range(first, last + 1)]
    width = args.width or max(1, int(max(font.getlength(ch) for ch in chars) + 0.999))
    height = args.height or (args.size + 2)
    row_stride = (width + 7) // 8
    line_height = args.line_height or height
    space_width = args.space_width or max(1, width // 2)

    if not (1 <= args.threshold <= 255):
        raise ValueError("--threshold must be between 1 and 255")
    if not (1 <= width <= 255 and 1 <= height <= 255):
        raise ValueError("glyph width and height must be between 1 and 255")
    if not (1 <= line_height <= 255 and 1 <= space_width <= 255):
        raise ValueError("line height and space width must be between 1 and 255")

    glyph_table = bytearray()
    bitmap_data = bytearray()

    for ch in chars:
        glyph = render_glyph(font, ch, width, height, args.threshold)
        packed, glyph_row_stride = pack_bitmap(glyph, width, height)
        if glyph_row_stride != row_stride:
            raise AssertionError("unexpected row stride")

        glyph_table += struct.pack("<BBBBI", width, height, row_stride, width, len(bitmap_data))
        bitmap_data += packed

    glyph_count = len(chars)
    glyph_table_offset = HEADER_SIZE
    bitmap_offset = glyph_table_offset + (glyph_count * GLYPH_SIZE)
    header = struct.pack(
        "<4sBBBBBBHIII",
        MAGIC,
        VERSION,
        first,
        last,
        line_height,
        space_width,
        0,
        glyph_count,
        glyph_table_offset,
        bitmap_offset,
        len(bitmap_data),
    )

    with open(args.output, "wb") as f:
        f.write(header)
        f.write(glyph_table)
        f.write(bitmap_data)


def main():
    parser = argparse.ArgumentParser(description="Convert a TTF/OTF font to an OpenMV .omvf bitmap font.")
    parser.add_argument("font", help="Input .ttf or .otf font")
    parser.add_argument("output", help="Output .omvf file")
    parser.add_argument("--size", type=int, default=16, help="Font point size")
    parser.add_argument("--width", type=int, default=0, help="Fixed glyph cell width in pixels")
    parser.add_argument("--height", type=int, default=0, help="Fixed glyph cell height in pixels")
    parser.add_argument("--line-height", type=int, default=0, help="Line height in pixels")
    parser.add_argument("--space-width", type=int, default=0, help="Advance for blank glyphs when mono_space=False")
    parser.add_argument("--first", default=" ", help="First character to include")
    parser.add_argument("--last", default="~", help="Last character to include")
    parser.add_argument("--threshold", type=int, default=128, help="Monochrome threshold, 1-255")
    convert(parser.parse_args())


if __name__ == "__main__":
    main()

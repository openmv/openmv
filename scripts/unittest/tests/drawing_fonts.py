def unittest(data_path, temp_path):
    import image

    def u16(value):
        return bytes((value & 0xFF, (value >> 8) & 0xFF))

    def u32(value):
        return bytes(
            (
                value & 0xFF,
                (value >> 8) & 0xFF,
                (value >> 16) & 0xFF,
                (value >> 24) & 0xFF,
            )
        )

    def make_font(path):
        first = 32
        last = ord("t")
        count = last - first + 1
        width = 4
        height = 6
        row_stride = 1
        bitmap = bytearray()
        glyphs = bytearray()

        for ch in range(first, last + 1):
            offset = len(bitmap)
            rows = [0x00] * height if ch == first else [0xF0] * height
            bitmap.extend(bytes(rows))
            glyphs.extend(bytes((width, height, row_stride, width)))
            glyphs.extend(u32(offset))

        glyph_offset = 24
        bitmap_offset = glyph_offset + (count * 8)
        header = (
            b"OMVF"
            + bytes((1, first, last, height, 2, 0))
            + u16(count)
            + u32(glyph_offset)
            + u32(bitmap_offset)
            + u32(len(bitmap))
        )

        with open(path, "wb") as f:
            f.write(header)
            f.write(glyphs)
            f.write(bitmap)

    default = image.Image(80, 30, image.GRAYSCALE)
    large = image.Image(80, 30, image.GRAYSCALE)
    loaded = image.Image(80, 30, image.GRAYSCALE)

    default.draw_string((0, 0), "Font")
    large.draw_string((0, 0), "Font", font=image.FONT_LARGE)
    font_path = temp_path + "/font.omvf"
    make_font(font_path)
    font = image.load_font(font_path)
    import gc
    gc.collect()
    loaded.draw_string((0, 0), "Font", font=font)

    default_pixels = 0
    large_pixels = 0
    loaded_pixels = 0
    for y in range(default.height()):
        for x in range(default.width()):
            default_pixels += 1 if default.get_pixel((x, y)) else 0
            large_pixels += 1 if large.get_pixel((x, y)) else 0
            loaded_pixels += 1 if loaded.get_pixel((x, y)) else 0

    try:
        large.draw_string((0, 0), "Font", font=99)
        return False
    except ValueError:
        pass

    # Reject truncated and invalid font data rather than reading outside the file.
    for contents in (b"", b"OMVF" + bytes(20)):
        with open(font_path, "wb") as f:
            f.write(contents)
        try:
            image.load_font(font_path)
            return False
        except ValueError:
            pass

    return (
        image.FONT_DEFAULT == image.FONT_8X10
        and image.FONT_LARGE == image.FONT_12X16
        and large_pixels > default_pixels
        and loaded_pixels > 0
    )

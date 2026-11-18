def unittest(data_path, temp_path):
    import image

    # Model of imlib_deyuv_line() for YUV422 (Y0 U Y1 V) to RGB565.
    def s8(x):
        return x - 256 if x & 0x80 else x

    def usat8(x):
        return 0 if x < 0 else (255 if x > 255 else x)

    def rgb565(y, ry, gy, by):
        r = usat8(y + ry)
        g = usat8(y - gy)
        b = usat8(y + by)
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

    def reference(data, w, h):
        out = []
        for y in range(h):
            row = y * w * 2
            for x in range(0, w, 2):
                if x >= w - 1:
                    # The last column of an odd width row reuses the previous chroma.
                    b = [data[row + 2 * x], data[row + 2 * x + 1],
                         data[row + 2 * x], data[row + 2 * (x - 1) + 1]]
                else:
                    b = data[row + 2 * x:row + 2 * x + 4]
                u = s8(b[1] ^ 0x80)
                v = s8(b[3] ^ 0x80)
                ry = (179 * u) >> 7
                gy = ((44 * v) + (91 * u)) >> 7
                by = (227 * v) >> 7
                out.append(rgb565(b[0], ry, gy, by))
                if x != w - 1:
                    out.append(rgb565(b[2], ry, gy, by))
        return out

    seed = [12345]

    def rand8():
        seed[0] = (seed[0] * 1103515245 + 12345) & 0x7FFFFFFF
        return (seed[0] >> 16) & 0xFF

    # Even, odd and short widths, with extreme values to hit the clamps.
    for w, h in ((64, 4), (37, 3), (8, 2), (3, 2), (2, 1)):
        data = bytearray(w * h * 2)
        for i in range(len(data)):
            data[i] = rand8()
        extremes = (0x00, 0xFF, 0x80, 0x10, 0xF0)
        for i in range(min(len(data), 40)):
            data[i] = extremes[i % len(extremes)]

        expected = reference(data, w, h)

        # Convert to a new image, and in place. The last column of an odd width row reads the
        # previous pixel, which is already converted in place, so only even widths are in place.
        for copy in (True, False):
            if not copy and (w % 2):
                continue
            img = image.Image(w, h, image.YUV422, buffer=bytearray(data))
            out = img.to_rgb565(copy=copy).bytearray()

            for i in range(w * h):
                if (out[2 * i] | (out[2 * i + 1] << 8)) != expected[i]:
                    return False

    return True

# GPU Stress / Correctness Test Suite
#
# Exercises image.draw_image(...) along every path the imlib_draw_image
# dispatcher (lib/imlib/draw.c) can take: the port's GPU driver (omv_gpu.c)
# where there is one, and the CPU path everywhere else. Each test is built
# around an *invariant* (identity, double-mirror, alpha=0, palette lookup,
# etc.) so it can self-validate without a CPU reference implementation.
#
# Each parametric path (alpha, scale, palette, dst-offset, src-size) is
# swept across multiple values so a regression in one corner doesn't hide
# behind a different corner that still works.
#
# Implementation notes:
#
#   * IMAGE_HINT_BLACK_BACKGROUND is intentionally not exposed for direct
#     user control (the constant overflows MicroPython's small-int packing
#     at bit 31 and reads as 0 from Python). It is set internally by the
#     dispatcher when needed, so we don't sweep it from the Python tests.
#
#   * The repeated_calls leak test is held at 50 iterations as a low-cost
#     canary against any future per-call resource leak in a GPU driver.
#     Long sessions used to hang the Alif GPU before the alphamode init
#     fix; if anything similar reappears this test will catch it.
#
# Runs two ways:
#   * scripts/unittest/run.py picks it up and calls unittest(), which prints
#     only failures and passes when every case passes.
#   * Run it from the IDE as a normal script to print every case. Edit the
#     run() call at the bottom (e.g. run(filter="alpha")) to run a subset.

import gc
import image
import uctypes


# ---------- helpers ---------------------------------------------------------

def _fill_rgb565(img, color):
    img.draw_rectangle(
        (0, 0, img.width(), img.height()), color=color, fill=True
    )


def _fill_gray(img, value):
    img.draw_rectangle(
        (0, 0, img.width(), img.height()), color=value, fill=True
    )


def _checker_gray(w, h, sq=8):
    img = image.Image(w, h, image.GRAYSCALE)
    for y in range(h):
        for x in range(w):
            v = 255 if ((x // sq) ^ (y // sq)) & 1 else 0
            img.set_pixel((x, y), v)
    return img


def _ramp_gray(w, h):
    img = image.Image(w, h, image.GRAYSCALE)
    for y in range(h):
        for x in range(w):
            img.set_pixel((x, y), (x + y) & 0xFF)
    return img


def _g(img, x, y):
    return img.get_pixel((x, y))


def _r(img, x, y):
    return img.get_pixel((x, y), rgbtuple=False)


def _eq_pix(a, b, tol=0):
    return abs(a - b) <= tol


def _approx_rgb565(a, b, tol=2):
    ar = (a >> 11) & 0x1F
    ag = (a >> 5) & 0x3F
    ab = a & 0x1F
    br = (b >> 11) & 0x1F
    bg = (b >> 5) & 0x3F
    bb = b & 0x1F
    return (abs(ar - br) <= tol
            and abs(ag - bg) <= tol
            and abs(ab - bb) <= tol)


def _rgb565_pack(r8, g8, b8):
    return ((r8 & 0xF8) << 8) | ((g8 & 0xFC) << 3) | (b8 >> 3)


# ---------- identity / pixfmt routing ---------------------------------------

def t_identity_rgb565():
    src = image.Image(64, 48, image.RGB565)
    _fill_rgb565(src, (200, 50, 100))
    src.draw_circle((32, 24, 10), color=(0, 255, 0), fill=True)
    dst = image.Image(64, 48, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0)
    for y in (0, 24, 47):
        for x in (0, 32, 63):
            if not _approx_rgb565(_r(src, x, y), _r(dst, x, y)):
                return False, "rgb565 mismatch at (%d,%d)" % (x, y)
    return True, ""


def t_identity_grayscale():
    src = _ramp_gray(64, 48)
    dst = image.Image(64, 48, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(src, 0, 0)
    for y in (0, 24, 47):
        for x in (0, 32, 63):
            sp = _g(src, x, y)
            dp = _g(dst, x, y)
            if not _eq_pix(sp, dp):
                return False, "gray mismatch (%d,%d) %d!=%d" % (x, y, sp, dp)
    return True, ""


def t_identity_grayscale_to_rgb565():
    src = _ramp_gray(32, 32)
    dst = image.Image(32, 32, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0)
    for y in (0, 16, 31):
        for x in (0, 16, 31):
            sp = _g(src, x, y)
            p = _r(dst, x, y)
            r = (p >> 11) & 0x1F
            g = (p >> 5) & 0x3F
            b = p & 0x1F
            if not (abs(r - (sp >> 3)) <= 1
                    and abs(g - (sp >> 2)) <= 1
                    and abs(b - (sp >> 3)) <= 1):
                return False, ("gray->rgb565 (%d,%d) src=%d got=0x%04X"
                               % (x, y, sp, p))
    return True, ""


def _identity_size_test(w, h):
    def t():
        src = _ramp_gray(w, h)
        dst = image.Image(w, h, image.GRAYSCALE)
        _fill_gray(dst, 0)
        dst.draw_image(src, 0, 0)
        for x, y in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1),
                     (w // 2, h // 2)):
            if not _eq_pix(_g(src, x, y), _g(dst, x, y)):
                return False, ("size %dx%d (%d,%d) %d!=%d"
                               % (w, h, x, y, _g(src, x, y), _g(dst, x, y)))
        return True, ""
    return t


def t_grayscale_to_grayscale_alpha8_path():
    src = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(src, 200)
    dst = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(src, 0, 0)
    m = dst.get_statistics().mean
    return (190 < m < 210), "gray->gray alpha8 mean=%d (want ~200)" % m


# ---------- alpha sweep -----------------------------------------------------
# Sweep alpha at 0, 32, 64, 96, 128, 160, 192, 224, 255 in two modes:
#   * RGB565 src + RGB565 dst, normal blend
#   * Grayscale src + RGB565 dst via flat palette (exercises the alpha8 +
#     CLUT path; alpha actually engages because there's a texel alpha)

def _alpha_rgb565_test(a):
    # Normal blend equation: out = src*a + dst*(1-a)
    # src = (255, 0, 0), dst = (0, 0, 255). Expected red = 31*a/255, blue = 31*(1-a/255).
    def t():
        src = image.Image(16, 16, image.RGB565)
        _fill_rgb565(src, (255, 0, 0))
        dst = image.Image(16, 16, image.RGB565)
        _fill_rgb565(dst, (0, 0, 255))
        dst.draw_image(src, 0, 0, alpha=a)
        p = _r(dst, 8, 8)
        r = (p >> 11) & 0x1F
        g = (p >> 5) & 0x3F
        b = p & 0x1F
        # Tolerance includes RGB565 truncation + dave2d rounding.
        exp_r = (31 * a + 127) // 255
        exp_b = (31 * (255 - a) + 127) // 255
        if abs(r - exp_r) > 3 or g > 2 or abs(b - exp_b) > 3:
            return False, ("alpha=%d got R=%d G=%d B=%d, expected R~%d B~%d"
                           % (a, r, g, b, exp_r, exp_b))
        return True, ""
    return t


def _alpha_grayscale_via_palette_test(a):
    # GPU restricts grayscale dst to alpha=255 + no palette. Test alpha
    # blending into RGB565 dst with grayscale src + identity-color palette.
    # That triggers the alpha8/CLUT path. Expected behavior: src=255,
    # dst=0, output mean = a (out of 255 -> map to 31 in the 5-bit channel).
    cpal = bytearray(256 * 2)
    for i in range(256):
        # White palette: every entry = 0xFFFF.
        cpal[i * 2 + 0] = 0xFF
        cpal[i * 2 + 1] = 0xFF
    cpal_img = image.Image(256, 1, image.RGB565, buffer=cpal)

    def t():
        src = image.Image(16, 16, image.GRAYSCALE)
        _fill_gray(src, 255)
        dst = image.Image(16, 16, image.RGB565)
        _fill_rgb565(dst, (0, 0, 0))
        dst.draw_image(src, 0, 0, alpha=a, color_palette=cpal_img)
        p = _r(dst, 8, 8)
        r = (p >> 11) & 0x1F
        # When src=255, gray value as alpha is 255. Combined with const
        # alpha a: effective = a. dst = src(white) * a/255 + 0 = a/255 white.
        exp_r = (31 * a + 127) // 255
        if abs(r - exp_r) > 3:
            return False, ("gray-via-palette a=%d R=%d expected ~%d"
                           % (a, r, exp_r))
        return True, ""
    return t


# ---------- mirror / flip with offsets and scaling --------------------------

def t_hmirror_corners():
    src = image.Image(32, 16, image.RGB565)
    _fill_rgb565(src, (0, 0, 0))
    src.set_pixel((0, 0), (255, 0, 0))
    src.set_pixel((31, 0), (0, 255, 0))
    dst = image.Image(32, 16, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0, hint=image.HMIRROR)
    ok = (_approx_rgb565(_r(dst, 0, 0), _r(src, 31, 0))
          and _approx_rgb565(_r(dst, 31, 0), _r(src, 0, 0)))
    return ok, "HMIRROR didn't swap horizontal corners"


def t_vflip_corners():
    src = image.Image(16, 32, image.RGB565)
    _fill_rgb565(src, (0, 0, 0))
    src.set_pixel((0, 0), (255, 0, 0))
    src.set_pixel((0, 31), (0, 255, 0))
    dst = image.Image(16, 32, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0, hint=image.VFLIP)
    ok = (_approx_rgb565(_r(dst, 0, 0), _r(src, 0, 31))
          and _approx_rgb565(_r(dst, 0, 31), _r(src, 0, 0)))
    return ok, "VFLIP didn't swap vertical corners"


def t_double_mirror_identity():
    src = _ramp_gray(32, 32)
    mid = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(mid, 0)
    out = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(out, 0)
    mid.draw_image(src, 0, 0, hint=image.HMIRROR)
    out.draw_image(mid, 0, 0, hint=image.HMIRROR)
    for y in (0, 15, 31):
        for x in (0, 15, 31):
            if not _eq_pix(_g(out, x, y), _g(src, x, y)):
                return False, "double HMIRROR not identity (%d,%d)" % (x, y)
    return True, ""


def t_hmirror_plus_vflip():
    src = image.Image(8, 8, image.RGB565)
    _fill_rgb565(src, (0, 0, 0))
    src.set_pixel((0, 0), (255, 0, 0))
    src.set_pixel((7, 7), (0, 255, 0))
    dst = image.Image(8, 8, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0, hint=image.HMIRROR | image.VFLIP)
    ok = (_approx_rgb565(_r(dst, 7, 7), _r(src, 0, 0))
          and _approx_rgb565(_r(dst, 0, 0), _r(src, 7, 7)))
    return ok, "HMIRROR|VFLIP didn't 180-rotate corners"


# ---------- in-place mirror/flip (same buffer src and dst) ------------------
# `crop(hint=...)`, `set(hint=...)`, and `draw_image(src=self, hint=...)` all
# pass the same buffer in. If the dispatcher doesn't deep-copy first, the
# iteration crosses the midpoint and starts reading its own writes,
# producing folded / doubled output.

def _inplace_mirror_test(name, hint, src_factory):
    def t():
        img = src_factory()
        # Compute expected by mirroring into a separate buffer first.
        ref = image.Image(img.width(), img.height(), image.RGB565)
        _fill_rgb565(ref, (0, 0, 0))
        ref.draw_image(img, 0, 0, hint=hint)
        # Now do the in-place op: same buffer for src and dst.
        img.draw_image(img, 0, 0, hint=hint)
        for x, y in ((0, 0), (img.width() - 1, 0),
                     (0, img.height() - 1),
                     (img.width() - 1, img.height() - 1),
                     (img.width() // 2, img.height() // 2)):
            if not _approx_rgb565(_r(img, x, y), _r(ref, x, y), tol=3):
                return False, ("%s in-place (%d,%d): got 0x%04X want 0x%04X"
                               % (name, x, y, _r(img, x, y), _r(ref, x, y)))
        return True, ""
    return t


def _make_corner_marked_rgb565(w, h):
    img = image.Image(w, h, image.RGB565)
    _fill_rgb565(img, (50, 50, 50))
    img.set_pixel((0, 0), (255, 0, 0))         # red TL
    img.set_pixel((w - 1, 0), (0, 255, 0))     # green TR
    img.set_pixel((0, h - 1), (0, 0, 255))     # blue BL
    img.set_pixel((w - 1, h - 1), (255, 255, 0))  # yellow BR
    return img


def t_inplace_crop_vflip():
    # The exact pattern from the user's bug report:
    #   csi.snapshot().crop(hint=image.VFLIP)
    # The image is the GPU's own framebuffer, src == dst, VFLIP requested.
    img = _make_corner_marked_rgb565(64, 48)
    # Capture original corners before in-place op.
    tl = _r(img, 0, 0)
    bl = _r(img, 0, 47)
    tr = _r(img, 63, 0)
    br = _r(img, 63, 47)
    img.crop(hint=image.VFLIP)
    # After VFLIP: top-left should hold what was bottom-left, etc.
    if not _approx_rgb565(_r(img, 0, 0), bl, tol=3):
        return False, ("crop VFLIP: (0,0)=0x%04X want 0x%04X (was bl)"
                       % (_r(img, 0, 0), bl))
    if not _approx_rgb565(_r(img, 0, 47), tl, tol=3):
        return False, ("crop VFLIP: (0,47)=0x%04X want 0x%04X (was tl)"
                       % (_r(img, 0, 47), tl))
    if not _approx_rgb565(_r(img, 63, 0), br, tol=3):
        return False, "crop VFLIP: (63,0) didn't get br"
    if not _approx_rgb565(_r(img, 63, 47), tr, tol=3):
        return False, "crop VFLIP: (63,47) didn't get tr"
    return True, ""


def t_inplace_crop_hmirror():
    img = _make_corner_marked_rgb565(64, 48)
    tl = _r(img, 0, 0)
    tr = _r(img, 63, 0)
    img.crop(hint=image.HMIRROR)
    if not _approx_rgb565(_r(img, 0, 0), tr, tol=3):
        return False, "crop HMIRROR: (0,0) didn't get tr"
    if not _approx_rgb565(_r(img, 63, 0), tl, tol=3):
        return False, "crop HMIRROR: (63,0) didn't get tl"
    return True, ""


def _mirror_with_offset_test(name, hint, ox, oy):
    # Place HMIRROR/VFLIP'd src into dst at non-zero offset and verify the
    # corners are mirrored AND positioned correctly.
    def t():
        src = image.Image(16, 16, image.RGB565)
        _fill_rgb565(src, (0, 0, 0))
        src.set_pixel((0, 0), (255, 0, 0))    # red TL
        src.set_pixel((15, 0), (0, 255, 0))   # green TR
        src.set_pixel((0, 15), (0, 0, 255))   # blue BL
        src.set_pixel((15, 15), (255, 255, 0))  # yellow BR
        dst = image.Image(64, 64, image.RGB565)
        _fill_rgb565(dst, (50, 50, 50))
        dst.draw_image(src, ox, oy, hint=hint)
        # After mirror, find where each src corner ended up.
        if hint & image.HMIRROR:
            tl_x = ox + 15
            tr_x = ox
        else:
            tl_x = ox
            tr_x = ox + 15
        if hint & image.VFLIP:
            tl_y = oy + 15
            bl_y = oy
        else:
            tl_y = oy
            bl_y = oy + 15
        # tl at (tl_x, tl_y) should be src(0,0) = red
        p = _r(dst, tl_x, tl_y)
        if not _approx_rgb565(p, 0xF800, tol=3):
            return False, ("%s: dst(%d,%d)=0x%04X want red"
                           % (name, tl_x, tl_y, p))
        # tr should be src(15,0) = green
        p = _r(dst, tr_x, tl_y)
        if not _approx_rgb565(p, 0x07E0, tol=3):
            return False, ("%s: dst(%d,%d)=0x%04X want green"
                           % (name, tr_x, tl_y, p))
        # bl should be src(0,15) = blue
        p = _r(dst, tl_x, bl_y)
        if not _approx_rgb565(p, 0x001F, tol=3):
            return False, ("%s: dst(%d,%d)=0x%04X want blue"
                           % (name, tl_x, bl_y, p))
        # Region outside the placed src must remain gray.
        if not _approx_rgb565(_r(dst, 0, 0), _rgb565_pack(50, 50, 50), tol=3):
            return False, ("%s: dst(0,0) clobbered" % name)
        return True, ""
    return t


# ---------- scaling sweep ---------------------------------------------------
# Scale a uniform src into dst at many ratios; result mean should still be
# the source value (modulo edge effects from bilinear). Catches src_rect
# computation errors, pitch overflow, interp setup bugs.

def _scale_uniform_test(name, sw, sh, dw, dh, hint=0, target=200):
    def t():
        src = image.Image(sw, sh, image.GRAYSCALE)
        _fill_gray(src, target)
        dst = image.Image(dw, dh, image.GRAYSCALE)
        _fill_gray(dst, 0)
        x_scale = float(dw) / sw
        y_scale = float(dh) / sh
        dst.draw_image(
            src, 0, 0, x_scale=x_scale, y_scale=y_scale, hint=hint
        )
        m = dst.get_statistics().mean
        if abs(m - target) > 10:
            return False, ("%s: mean=%d want ~%d (xs=%.4f ys=%.4f)"
                           % (name, m, target, x_scale, y_scale))
        return True, ""
    return t


def t_nearest_2x_upscale():
    src = image.Image(8, 8, image.GRAYSCALE)
    for y in range(8):
        for x in range(8):
            src.set_pixel((x, y), 255 if ((x ^ y) & 1) else 0)
    dst = image.Image(16, 16, image.GRAYSCALE)
    _fill_gray(dst, 128)
    dst.draw_image(src, 0, 0, x_scale=2.0, y_scale=2.0)
    for sy in range(8):
        for sx in range(8):
            expected = _g(src, sx, sy)
            for dy in range(2):
                for dx in range(2):
                    got = _g(dst, sx * 2 + dx, sy * 2 + dy)
                    if not _eq_pix(got, expected, tol=2):
                        return False, (
                            "nearest 2x: src(%d,%d)=%d dst(%d,%d)=%d"
                            % (sx, sy, expected,
                               sx * 2 + dx, sy * 2 + dy, got))
    return True, ""


def t_bilinear_2x_upscale_midpoint():
    src = image.Image(2, 2, image.GRAYSCALE)
    src.set_pixel((0, 0), 0)
    src.set_pixel((1, 0), 255)
    src.set_pixel((0, 1), 255)
    src.set_pixel((1, 1), 0)
    dst = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(dst, 200)
    dst.draw_image(
        src, 0, 0, x_scale=16.0, y_scale=16.0, hint=image.BILINEAR
    )
    mid = _g(dst, 15, 15)
    return (96 < mid < 160), "bilinear midpoint=%d (want ~127)" % mid


# ---------- forum issue: HD source, downscale to display --------------------
# https://forums.openmv.io/t/openmv-ae3-grayscale-issue-with-pag7936-in-hd/11599
# AE3 + PAG7936 HD = 1280x720 grayscale displayed at small dst. Verify
# both that the GPU handles it correctly and that aggressive downscales
# don't lose pixels.

def t_hd_grayscale_downscale_to_240():
    # Allocate ~900KB; skipped on boards without the memory.
    try:
        gc.collect()
        src = image.Image(1280, 720, image.GRAYSCALE)
    except MemoryError:
        return True, "skipped (no memory for 1280x720)"
    _fill_gray(src, 200)
    dst = image.Image(240, 135, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(
        src, 0, 0, x_scale=240.0 / 1280.0, y_scale=135.0 / 720.0
    )
    m = dst.get_statistics().mean
    src = None
    gc.collect()
    return abs(m - 200) < 10, "HD->240 grayscale mean=%d (want ~200)" % m


def t_hd_grayscale_downscale_to_64():
    try:
        gc.collect()
        src = image.Image(1280, 720, image.GRAYSCALE)
    except MemoryError:
        return True, "skipped (no memory for 1280x720)"
    _fill_gray(src, 200)
    dst = image.Image(64, 36, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(
        src, 0, 0, x_scale=64.0 / 1280.0, y_scale=36.0 / 720.0,
        hint=image.BILINEAR
    )
    m = dst.get_statistics().mean
    src = None
    gc.collect()
    return abs(m - 200) < 10, "HD->64 BILINEAR mean=%d (want ~200)" % m


# ---------- color & alpha palettes ------------------------------------------

def t_color_palette_lookup():
    pal = bytearray(256 * 2)
    target = 0x07FF
    pal[100 * 2 + 0] = target & 0xFF
    pal[100 * 2 + 1] = (target >> 8) & 0xFF
    pal_img = image.Image(256, 1, image.RGB565, buffer=pal)

    src = image.Image(8, 8, image.GRAYSCALE)
    _fill_gray(src, 100)
    dst = image.Image(8, 8, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0, color_palette=pal_img)

    p = _r(dst, 4, 4)
    return _approx_rgb565(p, target, tol=4), \
        "palette: dst pix=0x%04X (want ~0x%04X)" % (p, target)


def _palette_lookup_test(idx, rgb565):
    # Build a palette where only `idx` has `rgb565`, rest are black, then
    # blit a source filled with `idx` and verify dst pixel == rgb565.
    def t():
        pal = bytearray(256 * 2)
        pal[idx * 2 + 0] = rgb565 & 0xFF
        pal[idx * 2 + 1] = (rgb565 >> 8) & 0xFF
        pal_img = image.Image(256, 1, image.RGB565, buffer=pal)

        src = image.Image(8, 8, image.GRAYSCALE)
        _fill_gray(src, idx)
        dst = image.Image(8, 8, image.RGB565)
        _fill_rgb565(dst, (50, 50, 50))
        dst.draw_image(src, 0, 0, color_palette=pal_img)
        p = _r(dst, 4, 4)
        if not _approx_rgb565(p, rgb565, tol=4):
            return False, ("idx=%d pal=0x%04X got=0x%04X"
                           % (idx, rgb565, p))
        return True, ""
    return t


def t_alpha_palette_zero_preserves_dst():
    apal = bytearray(256)
    for i in range(256):
        apal[i] = 0 if i == 50 else 255
    apal_img = image.Image(256, 1, image.GRAYSCALE, buffer=apal)

    cpal = bytearray(256 * 2)
    for i in range(256):
        cpal[i * 2 + 0] = 0xFF
        cpal[i * 2 + 1] = 0xFF
    cpal_img = image.Image(256, 1, image.RGB565, buffer=cpal)

    src = image.Image(8, 8, image.GRAYSCALE)
    _fill_gray(src, 50)
    dst = image.Image(8, 8, image.RGB565)
    _fill_rgb565(dst, (0, 0, 255))
    pre = _r(dst, 4, 4)
    dst.draw_image(
        src, 0, 0, color_palette=cpal_img, alpha_palette=apal_img
    )
    post = _r(dst, 4, 4)
    return post == pre, \
        "alpha=0 entry didn't preserve dst (0x%04X -> 0x%04X)" % (pre, post)


def _alpha_palette_value_test(palette_alpha):
    # Build a flat alpha_palette of `palette_alpha`, white color palette,
    # src=255. Output should be ~ palette_alpha out of 255 white.
    def t():
        apal = bytearray(256)
        for i in range(256):
            apal[i] = palette_alpha
        apal_img = image.Image(256, 1, image.GRAYSCALE, buffer=apal)
        cpal = bytearray(256 * 2)
        for i in range(256):
            cpal[i * 2 + 0] = 0xFF
            cpal[i * 2 + 1] = 0xFF
        cpal_img = image.Image(256, 1, image.RGB565, buffer=cpal)

        src = image.Image(8, 8, image.GRAYSCALE)
        _fill_gray(src, 255)
        dst = image.Image(8, 8, image.RGB565)
        _fill_rgb565(dst, (0, 0, 0))
        dst.draw_image(
            src, 0, 0, color_palette=cpal_img, alpha_palette=apal_img
        )
        p = _r(dst, 4, 4)
        r = (p >> 11) & 0x1F
        exp_r = (31 * palette_alpha + 127) // 255
        if abs(r - exp_r) > 3:
            return False, ("apal=%d got R=%d want ~%d"
                           % (palette_alpha, r, exp_r))
        return True, ""
    return t


def t_palette_state_leak():
    pal = bytearray(256 * 2)
    for i in range(256):
        pal[i * 2 + 0] = 0xFF
        pal[i * 2 + 1] = 0xFF
    pal_img = image.Image(256, 1, image.RGB565, buffer=pal)

    gsrc = image.Image(8, 8, image.GRAYSCALE)
    _fill_gray(gsrc, 50)
    dst = image.Image(8, 8, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(gsrc, 0, 0, color_palette=pal_img)

    csrc = image.Image(8, 8, image.RGB565)
    _fill_rgb565(csrc, (10, 20, 30))
    dst2 = image.Image(8, 8, image.RGB565)
    _fill_rgb565(dst2, (0, 0, 0))
    dst2.draw_image(csrc, 0, 0)
    return _approx_rgb565(_r(dst2, 4, 4), _r(csrc, 4, 4)), \
        "stale CLUT bled into RGB->RGB draw"


# ---------- ROI, dst offset -------------------------------------------------

def t_dst_offset_no_overflow():
    src = image.Image(16, 16, image.RGB565)
    _fill_rgb565(src, (255, 0, 0))
    dst = image.Image(64, 64, image.RGB565)
    _fill_rgb565(dst, (0, 255, 0))
    dst.draw_image(src, 32, 32)

    if not _approx_rgb565(_r(dst, 0, 0), 0x07E0):
        return False, "(0,0) clobbered after offset draw"
    if not _approx_rgb565(_r(dst, 40, 40), 0xF800):
        return False, "(40,40) not red after offset draw"
    if not _approx_rgb565(_r(dst, 47, 47), 0xF800):
        return False, "(47,47) not red after offset draw (size truncated?)"
    spill = _r(dst, 50, 32)
    if not _approx_rgb565(spill, 0x07E0):
        return False, "red spilled past src extent at (50,32)=0x%04X" % spill
    return True, ""


def _dst_offset_test(ox, oy, sw, sh, dw, dh):
    # Drop a uniform red src into a green dst at offset (ox, oy). All
    # pixels inside the src footprint must be red, all outside green.
    def t():
        src = image.Image(sw, sh, image.RGB565)
        _fill_rgb565(src, (255, 0, 0))
        dst = image.Image(dw, dh, image.RGB565)
        _fill_rgb565(dst, (0, 255, 0))
        dst.draw_image(src, ox, oy)
        # Inside footprint
        if ox < dw and oy < dh:
            ix = min(ox + sw // 2, dw - 1)
            iy = min(oy + sh // 2, dh - 1)
            if not _approx_rgb565(_r(dst, ix, iy), 0xF800, tol=3):
                return False, ("inside (%d,%d) got 0x%04X"
                               % (ix, iy, _r(dst, ix, iy)))
        # Outside footprint top-left should still be green.
        if ox > 0 and oy > 0:
            if not _approx_rgb565(_r(dst, 0, 0), 0x07E0, tol=3):
                return False, ("outside (0,0) got 0x%04X"
                               % _r(dst, 0, 0))
        return True, ""
    return t


def t_roi_subrect():
    src = image.Image(32, 32, image.RGB565)
    _fill_rgb565(src, (0, 0, 0))
    src.draw_rectangle((8, 8, 16, 16), color=(255, 255, 255), fill=True)
    dst = image.Image(16, 16, image.RGB565)
    _fill_rgb565(dst, (0, 255, 0))
    dst.draw_image(src, 0, 0, roi=(8, 8, 16, 16))
    lm = dst.get_statistics().l_mean
    return lm > 90, "roi crop didn't pull white area, l_mean=%d" % lm


def _roi_test(rx, ry, rw, rh, sw=64, sh=64):
    # ROI sub-rect inside src. Region [rx..rx+rw, ry..ry+rh) is filled
    # white; rest of src is black. Blitting that ROI to dst should give
    # a white dst.
    def t():
        src = image.Image(sw, sh, image.RGB565)
        _fill_rgb565(src, (0, 0, 0))
        src.draw_rectangle((rx, ry, rw, rh),
                           color=(255, 255, 255), fill=True)
        dst = image.Image(rw, rh, image.RGB565)
        _fill_rgb565(dst, (255, 0, 0))
        dst.draw_image(src, 0, 0, roi=(rx, ry, rw, rh))
        lm = dst.get_statistics().l_mean
        if lm < 80:
            return False, ("roi (%d,%d,%d,%d) l_mean=%d"
                           % (rx, ry, rw, rh, lm))
        return True, ""
    return t


# ---------- combinations: alpha + scale + offset ----------------------------

def _combo_alpha_scale_test(a, scale):
    # Grayscale->grayscale on the Alif GPU requires alpha=255 + no palette.
    # Use grayscale src + RGB565 dst with no palette: the GPU loads an
    # identity CLUT (r=g=b=index, a=255). Filling
    # src with 255 makes the texel pure white, so the result depends
    # only on the constant alpha.
    def t():
        src = image.Image(32, 32, image.GRAYSCALE)
        _fill_gray(src, 255)
        dw = max(1, int(32 * scale))
        dh = dw
        dst = image.Image(dw, dh, image.RGB565)
        _fill_rgb565(dst, (0, 0, 0))
        dst.draw_image(
            src, 0, 0, x_scale=scale, y_scale=scale, alpha=a,
        )
        p = _r(dst, dw // 2, dh // 2)
        r = (p >> 11) & 0x1F
        exp_r = (31 * a + 127) // 255
        if abs(r - exp_r) > 4:
            return False, ("a=%d s=%.3f R=%d want ~%d"
                           % (a, scale, r, exp_r))
        return True, ""
    return t


def _combo_offset_scale_test(scale, ox, oy):
    # Place a uniform grayscale src at offset (ox,oy) in dst, scaled.
    # All pixels in the placed footprint must equal src; outside must
    # equal dst's prior value.
    def t():
        src = image.Image(16, 16, image.GRAYSCALE)
        _fill_gray(src, 200)
        dst = image.Image(64, 64, image.GRAYSCALE)
        _fill_gray(dst, 50)
        dst.draw_image(
            src, ox, oy, x_scale=scale, y_scale=scale
        )
        dw = int(16 * scale)
        # Center pixel of footprint
        cx = ox + dw // 2
        cy = oy + dw // 2
        if 0 <= cx < 64 and 0 <= cy < 64:
            v = _g(dst, cx, cy)
            if abs(v - 200) > 10:
                return False, ("center (%d,%d)=%d s=%.2f off=(%d,%d)"
                               % (cx, cy, v, scale, ox, oy))
        # Far corner outside footprint
        if ox + dw < 64 and oy + dw < 64:
            v = _g(dst, 63, 63)
            if abs(v - 50) > 5:
                return False, ("(63,63)=%d (should be 50, untouched)" % v)
        return True, ""
    return t


# ---------- boundaries / hardware limits ------------------------------------

def t_max_dst_size_just_under():
    src = image.Image(64, 64, image.RGB565)
    _fill_rgb565(src, (255, 255, 255))
    try:
        gc.collect()
        dst = image.Image(1023, 1023, image.RGB565)
    except MemoryError:
        return True, "skipped (no memory for 1023x1023)"
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(
        src, 0, 0,
        x_scale=1023.0 / 64.0, y_scale=1023.0 / 64.0,
        hint=image.BILINEAR,
    )
    p = _r(dst, 500, 500)
    return _approx_rgb565(p, 0xFFFF, tol=4), \
        "1023x1023 GPU draw produced 0x%04X at center" % p


def t_max_src_at_2048_wide():
    # Just under the 2048 source-width limit (dave2d). Blit a 1px slice.
    try:
        gc.collect()
        src = image.Image(2048, 1, image.GRAYSCALE)
    except MemoryError:
        return True, "skipped (no memory for 2048x1)"
    _fill_gray(src, 175)
    dst = image.Image(64, 1, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(src, 0, 0, x_scale=64.0 / 2048.0, y_scale=1.0)
    m = dst.get_statistics().mean
    src = None
    gc.collect()
    return abs(m - 175) < 10, "2048-wide src mean=%d want ~175" % m


def t_oversize_src_falls_back():
    # 2049x1 exceeds the 2048 source-width limit (dave2d); must fall back
    # to CPU instead of raising.
    try:
        gc.collect()
        src = image.Image(2049, 1, image.GRAYSCALE)
    except MemoryError:
        return True, "skipped (no memory for 2049x1)"
    _fill_gray(src, 175)
    dst = image.Image(64, 1, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(src, 0, 0, x_scale=64.0 / 2049.0, y_scale=1.0)
    m = dst.get_statistics().mean
    src = None
    gc.collect()
    return abs(m - 175) < 10, "2049-wide fallback mean=%d want ~175" % m


def t_tall_src_falls_back():
    # 100x1025 exceeds the 1024 source-height limit (dave2d); fall back to CPU.
    try:
        gc.collect()
        src = image.Image(100, 1025, image.GRAYSCALE)
    except MemoryError:
        return True, "skipped (no memory for 100x1025)"
    _fill_gray(src, 175)
    dst = image.Image(50, 50, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(src, 0, 0, x_scale=0.5, y_scale=50.0 / 1025.0)
    m = dst.get_statistics().mean
    src = None
    gc.collect()
    return abs(m - 175) < 10, "1025-tall fallback mean=%d want ~175" % m


def t_unsupported_hint_falls_back():
    src = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(src, 200)
    dst = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(dst, 0)
    dst.draw_image(src, 0, 0, hint=image.TRANSPOSE)
    m = dst.get_statistics().mean
    return m > 180, "TRANSPOSE fallback mean=%d (expected ~200)" % m


# ---------- robustness / state leak -----------------------------------------

def t_repeated_calls():
    # Per-call leak canary; see the implementation notes at the top.
    src = _ramp_gray(64, 64)
    dst = image.Image(64, 64, image.GRAYSCALE)
    _fill_gray(dst, 0)
    for _ in range(50):
        dst.draw_image(src, 0, 0)
    return _eq_pix(_g(dst, 32, 32), _g(src, 32, 32), tol=1), \
        "after 50 redraws final pixel diverged"


def t_back_to_back_format_changes():
    g = _ramp_gray(32, 32)
    r = image.Image(32, 32, image.RGB565)
    _fill_rgb565(r, (200, 100, 50))
    out_g = image.Image(32, 32, image.GRAYSCALE)
    _fill_gray(out_g, 0)
    out_r = image.Image(32, 32, image.RGB565)
    _fill_rgb565(out_r, (0, 0, 0))
    out_g.draw_image(g, 0, 0)
    out_r.draw_image(r, 0, 0)
    out_r.draw_image(g, 0, 0)
    if not _eq_pix(_g(out_g, 16, 16), _g(g, 16, 16), tol=1):
        return False, "gray-out wrong after fmt switch"
    return True, ""


def t_every_heap_region():
    # The GC fills its heap blocks in order, so keep allocating until images
    # land in every region it hands out (e.g. SRAM4 on the H7, which DMA2D
    # can't reach), then blit between every pair of them.
    regions = {}
    fill = []
    try:
        for _ in range(512):
            img = image.Image(16, 16, image.RGB565)
            imgs = regions.setdefault(uctypes.addressof(img.bytearray()) >> 24, [])
            if len(imgs) < 2:
                imgs.append(img)
            else:
                fill.append(img)
    except MemoryError:
        pass
    fill = None
    gc.collect()
    imgs = [img for r in regions.values() for img in r]
    for src in imgs:
        for dst in imgs:
            if src is dst:
                continue
            _fill_rgb565(src, (255, 0, 0))
            _fill_rgb565(dst, (0, 255, 0))
            dst.draw_image(src, 0, 0)
            if _r(dst, 8, 8) != _r(src, 8, 8):
                return False, ("blit 0x%08X -> 0x%08X got 0x%04X"
                               % (uctypes.addressof(src.bytearray()),
                                  uctypes.addressof(dst.bytearray()), _r(dst, 8, 8)))
    return True, ""


def t_state_mix_alpha_palette_mirror():
    # Walk every state-changing knob in sequence. If any of them leaves
    # stale state in the GPU context, a later test will see it.
    src = image.Image(16, 16, image.RGB565)
    _fill_rgb565(src, (10, 200, 30))
    dst = image.Image(32, 32, image.RGB565)
    _fill_rgb565(dst, (0, 0, 0))
    pal = bytearray(256 * 2)
    for i in range(256):
        pal[i * 2 + 0] = 0xFF
        pal[i * 2 + 1] = 0xFF
    pal_img = image.Image(256, 1, image.RGB565, buffer=pal)
    gsrc = image.Image(16, 16, image.GRAYSCALE)
    _fill_gray(gsrc, 128)

    # Sequence: alpha blend, mirror, scale, palette, transpose-fallback,
    # then a plain identity blit. The final identity must produce exact
    # source pixels.
    dst.draw_image(src, 0, 0, alpha=128)
    dst.draw_image(src, 0, 0, hint=image.HMIRROR)
    dst.draw_image(src, 0, 0, x_scale=1.5, y_scale=1.5)
    dst.draw_image(gsrc, 0, 0, color_palette=pal_img)
    dst.draw_image(src, 0, 0, hint=image.TRANSPOSE)  # CPU fallback
    _fill_rgb565(dst, (0, 0, 0))
    dst.draw_image(src, 0, 0)  # plain identity
    p = _r(dst, 8, 8)
    sp = _r(src, 8, 8)
    if not _approx_rgb565(p, sp, tol=3):
        return False, ("post-mix identity got 0x%04X want 0x%04X" % (p, sp))
    return True, ""


# ---------- registry --------------------------------------------------------

def _build_tests():
    tests = [
        # Identity / pixfmt routing
        ("identity_rgb565", t_identity_rgb565),
        ("identity_grayscale", t_identity_grayscale),
        ("identity_grayscale_to_rgb565", t_identity_grayscale_to_rgb565),
        ("gray_to_gray_alpha8_path", t_grayscale_to_grayscale_alpha8_path),
    ]

    # Identity at various sizes -- catches stride/pitch quirks.
    for w, h in ((1, 1), (2, 2), (3, 3), (8, 8), (16, 17), (33, 33),
                 (128, 128), (256, 7)):
        tests.append(
            ("identity_size_%dx%d" % (w, h), _identity_size_test(w, h))
        )

    # Alpha sweep -- normal blend
    for a in (0, 32, 64, 96, 128, 160, 192, 224, 255):
        tests.append(
            ("alpha_blend_a%d" % a, _alpha_rgb565_test(a))
        )

    # Alpha sweep via grayscale + flat palette
    for a in (32, 64, 128, 192, 255):
        tests.append(
            ("alpha_via_palette_a%d" % a,
             _alpha_grayscale_via_palette_test(a))
        )

    # Mirror / flip
    tests.extend([
        ("hmirror_corners", t_hmirror_corners),
        ("vflip_corners", t_vflip_corners),
        ("double_mirror_identity", t_double_mirror_identity),
        ("hmirror_plus_vflip", t_hmirror_plus_vflip),
    ])

    # In-place mirror/flip (src == dst). Without a deep-copy of src, the
    # iteration reads its own writes after crossing the midpoint. This is
    # the failure mode that produced the doubled/folded image from
    # csi.snapshot().crop(hint=image.VFLIP) in the field.
    tests.extend([
        ("inplace_crop_vflip", t_inplace_crop_vflip),
        ("inplace_crop_hmirror", t_inplace_crop_hmirror),
    ])
    for name, hint in (
        ("inplace_draw_vflip", image.VFLIP),
        ("inplace_draw_hmirror", image.HMIRROR),
        ("inplace_draw_vflip_hmirror", image.VFLIP | image.HMIRROR),
    ):
        tests.append(
            (name, _inplace_mirror_test(
                name, hint, lambda: _make_corner_marked_rgb565(64, 48)))
        )

    # Mirror with offset (catches dispatcher dst_rect / src_rect math when
    # mirror is combined with non-zero placement).
    for name, hint, ox, oy in (
        ("hmirror_off_8_0", image.HMIRROR, 8, 0),
        ("hmirror_off_0_8", image.HMIRROR, 0, 8),
        ("vflip_off_8_8", image.VFLIP, 8, 8),
        ("hmirror_vflip_off_16_16",
         image.HMIRROR | image.VFLIP, 16, 16),
    ):
        tests.append(
            (name, _mirror_with_offset_test(name, hint, ox, oy))
        )

    # Scale sweep -- upscales with both nearest and bilinear.
    upscale_cases = (
        ("up_1.10x", 64, 64, 70, 70, 0),
        ("up_1.50x", 32, 32, 48, 48, 0),
        ("up_2.00x", 32, 32, 64, 64, 0),
        ("up_4.00x", 16, 16, 64, 64, 0),
        ("up_8.00x", 16, 16, 128, 128, 0),
        ("up_16.0x", 16, 16, 256, 256, 0),
        ("up_2.00x_bilinear", 32, 32, 64, 64, image.BILINEAR),
        ("up_4.00x_bilinear", 16, 16, 64, 64, image.BILINEAR),
        ("up_8.00x_bilinear", 16, 16, 128, 128, image.BILINEAR),
    )
    for name, sw, sh, dw, dh, hint in upscale_cases:
        tests.append(
            ("scale_%s" % name,
             _scale_uniform_test(name, sw, sh, dw, dh, hint))
        )

    # Scale sweep -- downscales (the forum-issue regime).
    downscale_cases = (
        ("dn_0.99x", 100, 100, 99, 99, 0),
        ("dn_0.75x", 64, 64, 48, 48, 0),
        ("dn_0.50x", 64, 64, 32, 32, 0),
        ("dn_0.25x", 128, 128, 32, 32, 0),
        ("dn_0.10x", 320, 240, 32, 24, 0),
        ("dn_0.05x", 640, 480, 32, 24, 0),
        ("dn_0.50x_bilinear", 64, 64, 32, 32, image.BILINEAR),
        ("dn_0.25x_bilinear", 128, 128, 32, 32, image.BILINEAR),
        ("dn_0.10x_bilinear", 320, 240, 32, 24, image.BILINEAR),
    )
    for name, sw, sh, dw, dh, hint in downscale_cases:
        tests.append(
            ("scale_%s" % name,
             _scale_uniform_test(name, sw, sh, dw, dh, hint))
        )

    # Scaling: bilinear midpoint sanity
    tests.extend([
        ("nearest_2x_upscale", t_nearest_2x_upscale),
        ("bilinear_2x_upscale_midpoint", t_bilinear_2x_upscale_midpoint),
    ])

    # Forum-issue scenarios: HD source downscaled.
    tests.extend([
        ("hd_grayscale_to_240x135", t_hd_grayscale_downscale_to_240),
        ("hd_grayscale_to_64x36", t_hd_grayscale_downscale_to_64),
    ])

    # Color palette lookups at multiple indices and colors.
    palette_cases = (
        (0, 0xF800),    # red at index 0
        (1, 0x07E0),    # green at index 1 (catches off-by-one)
        (64, 0x001F),   # blue at index 64
        (127, 0xFFE0),  # yellow at midpoint
        (128, 0x07FF),  # cyan crossing the midpoint
        (200, 0xF81F),  # magenta near top
        (254, 0x4208),  # mid-gray near top (catches mask bugs)
        (255, 0xFFFF),  # white at last index
    )
    for idx, color in palette_cases:
        tests.append(
            ("palette_idx_%d" % idx, _palette_lookup_test(idx, color))
        )

    # Alpha palette sweep
    tests.append(("alpha_palette_zero_preserves",
                  t_alpha_palette_zero_preserves_dst))
    for pa in (32, 96, 160, 224):
        tests.append(
            ("alpha_palette_flat_a%d" % pa, _alpha_palette_value_test(pa))
        )

    tests.append(("palette_state_leak", t_palette_state_leak))

    # Dst offset matrix
    tests.append(("dst_offset_no_overflow", t_dst_offset_no_overflow))
    for ox, oy, sw, sh, dw, dh in (
        (1, 0, 16, 16, 64, 64),
        (0, 1, 16, 16, 64, 64),
        (1, 1, 16, 16, 64, 64),
        (32, 32, 16, 16, 64, 64),
        (47, 47, 16, 16, 64, 64),
        (48, 48, 16, 16, 64, 64),
        (60, 60, 16, 16, 64, 64),
    ):
        tests.append(
            ("dst_offset_%d_%d" % (ox, oy),
             _dst_offset_test(ox, oy, sw, sh, dw, dh))
        )

    # ROI sub-rects -- each picks a different aligned/unaligned region.
    tests.append(("roi_subrect", t_roi_subrect))
    for rx, ry, rw, rh in (
        (0, 0, 8, 8),
        (1, 1, 8, 8),
        (16, 0, 16, 16),
        (0, 16, 16, 16),
        (24, 24, 8, 8),
        (32, 32, 16, 16),  # ROI right at corner
    ):
        tests.append(
            ("roi_%d_%d_%d_%d" % (rx, ry, rw, rh),
             _roi_test(rx, ry, rw, rh))
        )

    # Combination: alpha + scale (palette path so alpha actually engages).
    for a in (64, 128, 192):
        for s in (0.5, 1.0, 2.0, 4.0):
            tests.append(
                ("combo_a%d_s%.2f" % (a, s),
                 _combo_alpha_scale_test(a, s))
            )

    # Combination: offset + scale
    for s, ox, oy in (
        (0.5, 8, 8),
        (1.0, 8, 8),
        (2.0, 8, 8),
        (1.5, 16, 16),
    ):
        tests.append(
            ("combo_off_%d_%d_s%.2f" % (ox, oy, s),
             _combo_offset_scale_test(s, ox, oy))
        )

    # Boundaries
    tests.extend([
        ("max_dst_size_1023", t_max_dst_size_just_under),
        ("max_src_2048_wide", t_max_src_at_2048_wide),
        ("oversize_src_falls_back", t_oversize_src_falls_back),
        ("tall_src_falls_back", t_tall_src_falls_back),
        ("unsupported_hint_falls_back", t_unsupported_hint_falls_back),
    ])

    # Robustness
    tests.extend([
        ("repeated_calls", t_repeated_calls),
        ("back_to_back_format_changes", t_back_to_back_format_changes),
        ("state_mix_alpha_palette_mirror", t_state_mix_alpha_palette_mirror),
        ("every_heap_region", t_every_heap_region),
    ])

    return tests


def run(filter=None, verbose=True):
    passed = 0
    skipped = 0
    failed = 0
    failures = []
    for name, fn in _build_tests():
        if filter is not None and filter not in name:
            continue
        try:
            ok, msg = fn()
        except MemoryError:
            # Boards with little RAM can't fit every case.
            ok, msg = True, "skipped (no memory)"
        except Exception as e:
            ok = False
            msg = "EXCEPTION: %r" % e
        skip = ok and msg.startswith("skipped")
        tag = "SKIP" if skip else "PASS" if ok else "FAIL"
        suffix = ("  -- " + msg) if (msg and not ok) else ""
        if verbose or not ok:
            print("%-40s %s%s" % (name, tag, suffix))
        if skip:
            skipped += 1
        elif ok:
            passed += 1
        else:
            failed += 1
            failures.append(name)
        gc.collect()
    if verbose:
        print("---")
        print("%d passed, %d skipped, %d failed" % (passed, skipped, failed))
        if failures:
            print("failed: %s" % ", ".join(failures))
    return failed


def unittest(data_path, temp_path):
    return run(verbose=False) == 0


# run.py exec()s this file in its own globals, where __name__ is also "__main__".
if "TEST_PATH" not in globals():
    run()

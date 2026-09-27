#!/usr/bin/env python3
"""
grab_frame.py -- save one raw camera frame from the cashew firmware as images.

    pip install pyserial
    python grab_frame.py COM5            (Windows)
    python grab_frame.py /dev/ttyACM0    (Linux; macOS: /dev/cu.usbmodem...)

Sends "dump" to the board, reads the hex frame and writes
    frame_raw.pgm  -- the raw Bayer mosaic (8-bit, as the detector sees it)
    frame_rgb.ppm  -- a simple half-resolution colour preview (2x2 Bayer cell -> 1 pixel)
Open them with GIMP, IrfanView, Photoshop, or Python (PIL / OpenCV).
The detection loop is paused during the transfer (~2 s) and resumes afterwards.
"""
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("pyserial is missing: pip install pyserial")

# Colour of each pixel of a 2x2 cell for the firmware's cfa codes 0..3:
# index = (y & 1) * 2 + (x & 1); 0 = R, 1 = G, 2 = B.
CFA = {0: (2, 1, 1, 0), 1: (1, 2, 0, 1), 2: (1, 0, 2, 1), 3: (0, 1, 1, 2)}


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    port = sys.argv[1]
    baud = int(sys.argv[2]) if len(sys.argv) > 2 else 921600
    s = serial.Serial(port, baud, timeout=5)
    s.reset_input_buffer()
    s.write(b"dump\r\n")
    w = h = cfa = None
    t0 = time.time()
    while time.time() - t0 < 10:
        line = s.readline().decode(errors="replace").strip()
        if line.startswith("FRAME"):
            p = line.split()
            w, h, cfa = int(p[1]), int(p[2]), int(p[4])
            break
    if w is None:
        sys.exit("no FRAME header received (is the firmware running and the port right?)")
    rows = []
    while len(rows) < h:
        line = s.readline().decode(errors="replace").strip()
        if not line:
            sys.exit("timeout after %d of %d rows" % (len(rows), h))
        if len(line) != 2 * w:
            continue            # a log line mixed in: skip it
        rows.append(bytes.fromhex(line))
    raw = b"".join(rows)
    with open("frame_raw.pgm", "wb") as f:
        f.write(b"P5\n%d %d\n255\n" % (w, h))
        f.write(raw)
    rgb = bytearray()
    m = CFA.get(cfa, CFA[2])
    for y in range(0, h - 1, 2):
        for x in range(0, w - 1, 2):
            c = [0, 0, 0]
            n = [0, 0, 0]
            for k, (dy, dx) in enumerate(((0, 0), (0, 1), (1, 0), (1, 1))):
                ch = m[k]
                c[ch] += raw[(y + dy) * w + x + dx]
                n[ch] += 1
            rgb += bytes(min(255, c[i] // max(1, n[i])) for i in range(3))
    with open("frame_rgb.ppm", "wb") as f:
        f.write(b"P6\n%d %d\n255\n" % (w // 2, h // 2))
        f.write(rgb)
    mean = sum(raw) / len(raw)
    print("saved frame_raw.pgm (%dx%d) and frame_rgb.ppm (%dx%d), cfa %d, mean %.1f"
          % (w, h, w // 2, h // 2, cfa, mean))


if __name__ == "__main__":
    main()

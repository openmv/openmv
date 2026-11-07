# Bitmap fonts

`image.draw_string()` uses the existing 8x10 font by default. Pass
`font=image.FONT_12X16` to use the larger built-in bitmap font, or load a
custom font from a file:

```python
import image

font = image.load_font("/sd/myfont.omvf")
img = image.Image(320, 240, image.RGB565)
img.draw_string((10, 10), "Hello", font=font, color=(255, 255, 255))
```

Create a monochrome bitmap font on a computer using Python and Pillow:

```sh
python -m pip install Pillow
python tools/font2omvf.py path/to/font.ttf myfont.omvf --size 24
```

Copy the resulting file to the camera. Run `python tools/font2omvf.py --help`
for cell dimensions, spacing, threshold, and character range options. The
converter supports ASCII characters; the range must include the space character.
The cells have fixed dimensions, and glyphs that extend beyond them are clipped;
use `--width` and `--height` to adjust them if needed. Loading from ordinary
storage copies the font into MicroPython heap memory; memory-mapped files can be
used directly.

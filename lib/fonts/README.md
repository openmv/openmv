# Fonts

TrueType fonts shipped in the firmware's ROMFS for `image.draw_string()` and `image.Font()`.

Roboto is proportional; Roboto Mono is fixed pitch (every glyph has the same advance), for
columns, consoles and anything laid out by counting characters. `image.draw_string()` uses
Roboto Mono Regular at 10 px by default. Boards with a 128 or 256 KB ROMFS (OPENMV2/3/4) carry
only the ASCII subset of that face, the 4 MB Arduino boards carry the four Roboto Mono faces, and
boards with 8 MB or more carry all eight faces.

The files here are the unmodified upstream fonts. At build time `tools/mkromfs.py` subsets each
one to the code points listed in the board's `romfs_config.json` (`"type": "ttf"` entries) with
`tools/mkfont.py`, which drops hinting, kerning, names and every other table the firmware does
not read. Only `cmap`, `glyf`, `head`, `hhea`, `hmtx`, `loca` and `maxp` are kept.

```json
{ "type": "ttf", "path": "{TOP}/lib/fonts/RobotoMono-Regular.ttf", "unicodes": "U+0020-007E" }
```

`unicodes` is a comma separated list of code points or ranges; `text` may be added to keep the
characters of a literal string, and `text_file` names a UTF-8 file whose characters are kept
(lines starting with `#` are comments). The same tool subsets any TrueType font for user ROMFS images:

`ascender` and `descender` (in ems, given together) replace the font's line metrics. The Roboto
Mono entries use 0.8 and 0.2, which makes a line of text exactly as tall as the font size with the
baseline at 80% of it. At the default 10 px that is a 6x10 pixel cell with the baseline 8 pixels
down, so text drawn with the default font stays inside the 8x10 cell of the bitmap font it
replaced and lines are 10 pixels apart. Roboto and the Droid fonts keep their own, looser spacing.

```
python3 tools/mkfont.py MyFont.ttf MyFont.subset.ttf --unicodes U+0020-007E,U+00A0-00FF --text "€✓"
```

Only TrueType outlines (`glyf`) are supported. CFF/OpenType (`.otf`) and TrueType collections
(`.ttc`) are rejected.

| File | Source | License |
|---|---|---|
| `Roboto-Regular.ttf`, `Roboto-Bold.ttf`, `Roboto-Italic.ttf`, `Roboto-BoldItalic.ttf` | https://github.com/googlefonts/roboto (v2.138, unhinted) | Apache License 2.0, see `LICENSE-Roboto` |
| `RobotoMono-Regular.ttf`, `RobotoMono-Bold.ttf`, `RobotoMono-Italic.ttf`, `RobotoMono-BoldItalic.ttf` | https://github.com/googlefonts/RobotoMono (commit 7bf0b33) | SIL Open Font License 1.1, see `LICENSE-RobotoMono` |
| `DroidSansFallback.ttf`, `DroidNaskh-Regular.ttf` | Android Open Source Project, frameworks/base/data/fonts (android-4.4_r1) | Apache License 2.0, see `LICENSE-Droid` |

## Chinese, Japanese and Korean

Boards with a large ROMFS (OPENMV_N6, OPENMV_AE3) also ship `DroidSansFallback.ttf`, Android's
CJK fallback font, subset to about 790 KB with the character list in `cjk_common.txt`: kana,
CJK punctuation and fullwidth forms, the 2500 most frequent Chinese characters, the 2136 joyo
kanji and the 2350 KS X 1001 hangul syllables. Load it with `image.Font("/rom/DroidSansFallback.ttf", 16)`;
ideographs need 16 px or more to be legible. Characters outside the list draw as a box; edit
`cjk_common.txt` (or the board's `romfs_config.json`) to change the set.

## Arabic

The same boards ship `DroidNaskh-Regular.ttf` (Arabic, Persian and Urdu letters, about 31 KB).
The firmware shapes Arabic itself: letters take their initial, medial, final or isolated form
from the font's Unicode presentation forms, lam-alef becomes its ligature, vowel marks stay on
their letter, and Arabic runs are drawn right to left with numbers left to right inside them.
Pass the text in logical order, as typed; nothing else is needed. Any font that carries the
Arabic Presentation Forms-B block works the same way.

## Fallback fonts

`image.Font(path, size, fallback=other_font)` draws characters the font lacks with `other_font`
(which may have a fallback of its own). Without `fallback` a font falls back to the default font.
Which ROMFS font is the default and which ones it falls back to is set in the board's
`romfs_config.json`: the `ttf` entry marked `"default": true` and, in order, the entries marked
`"fallback": true`; `mkromfs.py` lists them in `/rom/fonts.txt` for the firmware. So
`draw_string()` with no font draws Chinese, Japanese, Korean and Arabic out of the box, and
`DroidNaskh-Regular.ttf`, which has no Latin letters or digits at all, gets them from the default.

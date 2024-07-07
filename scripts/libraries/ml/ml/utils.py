# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

def draw_predictions(
    image,
    boxes,
    labels,
    colors,
    format="pascal_voc",
    font_width=8,
    font_height=10,
    text_color=(255, 255, 255),
):
    image_w = image.width()
    image_h = image.height()
    for i, (x, y, w, h) in enumerate(boxes):
        label = labels[i]
        box_color = colors[i]

        if format == "pascal_voc":
            x = int(x * image_w)
            y = int(y * image_h)
            w = int(w * image_w) - x
            h = int(h * image_h) - y

        image.draw_rectangle(x, y, w, h, color=box_color)
        image.draw_rectangle(
            x,
            y - font_height,
            len(label) * font_width,
            font_height,
            fill=True,
            color=box_color,
        )
        image.draw_string(x, y - font_height, label.upper(), text_color)

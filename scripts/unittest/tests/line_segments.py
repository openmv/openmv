def unittest(data_path, temp_path):
    import image

    img = image.Image(data_path+"/shapes.ppm", copy_to_fb=True)
    lines = img.find_line_segments()
    return (
        len(lines) == 6
        and lines[0][0:] == (24, 74, 56, 74, 32, 18, 90, 74)
        and lines[1][0:] == (54, 38, 26, 38, 28, 13, 90, 38)
        and lines[2][0:] == (104, 70, 114, 76, 12, 2, 121, 6)
        and lines[3][0:] == (139, 51, 133, 41, 12, 2, 149, -93)
        and lines[4][0:] == (109, 37, 100, 46, 13, 14, 45, 103)
        and lines[5][0:] == (129, 73, 137, 64, 12, 6, 42, 145)
    )

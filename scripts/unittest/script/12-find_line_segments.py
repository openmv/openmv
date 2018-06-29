def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/shapes.ppm", copy_to_fb=True)
    lines = img.find_line_segments()
    return len(lines) == 7 and\
    lines[0][0:] == (23, 58, 22, 58, 1, 16, 90, 58) and\
    lines[1][0:] == (24, 74, 56, 74, 32, 19, 90, 74) and\
    lines[2][0:] == (54, 38, 26, 38, 28, 14, 90, 38) and\
    lines[3][0:] == (104, 70, 114, 76, 12, 2, 121, 6) and\
    lines[4][0:] == (139, 51, 133, 41, 12, 2, 149, -93) and\
    lines[5][0:] == (109, 37, 100, 46, 13, 12, 45, 103) and\
    lines[6][0:] == (127, 75, 138, 64, 16, 1, 45, 143)

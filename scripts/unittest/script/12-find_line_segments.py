def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/shapes.ppm", copy_to_fb=True)
    lines = img.find_line_segments(threshold = 10000, theta_margin = 15, rho_margin = 15, segment_threshold = 100)
    return len(lines) == 6 and\
    lines[0][0:] == (56, 38, 24, 38, 32, 19, 90, 38)    and\
    lines[1][0:] == (26, 74, 54, 74, 28, 14, 90, 74)    and\
    lines[2][0:] == (104, 70, 114, 76, 12, 2, 121, 6)   and\
    lines[3][0:] == (139, 51, 133, 41, 12, 2, 149, -93) and\
    lines[4][0:] == (109, 37, 100, 46, 13, 12, 45, 103) and\
    lines[5][0:] == (127, 75, 138, 64, 16, 1, 45, 143)







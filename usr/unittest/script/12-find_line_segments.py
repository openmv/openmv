def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/shapes.ppm", copy_to_fb=True)
    lines = img.find_line_segments(threshold = 10000, theta_margin = 15, rho_margin = 15, segment_threshold = 100)
    return len(lines) == 6 and\
    lines[0][0:] ==(22, 40, 22, 73, 33, 17340, 0, 22)    and\
    lines[1][0:] ==(111, 37, 124, 37, 13, 17340, 90, 39) and\
    lines[2][0:] ==(24, 39, 56, 39, 32, 17340, 90, 39)   and\
    lines[3][0:] ==(57, 40, 57, 73, 33, 17340, 0, 57)    and\
    lines[4][0:] ==(24, 75, 56, 75, 32, 21420, 90, 75)   and\
    lines[5][0:] ==(112, 75, 126, 75, 14, 21420, 90, 75)









def unittest(data_path, temp_path):
    import image

    img = image.Image(data_path+"/shapes.ppm", copy_to_fb=True)
    all_lines = img.find_line_segments()

    # Filter out spurious short lines (Unix port may detect 1-pixel segments)
    # All valid lines have length >= 12
    lines = [line for line in all_lines if line.length() >= 10]

    # Helper to check line parameters with tolerance for math approximation differences
    def close_match(actual_tuple, expected_tuple, tolerance=2):
        if len(actual_tuple) != len(expected_tuple):
            return False
        for a, e in zip(actual_tuple, expected_tuple):
            if abs(a - e) > tolerance:
                return False
        return True

    # Expected line parameters: (x1, y1, x2, y2, length, magnitude, theta, rho)
    expected = [
        (24, 74, 56, 74, 32, 18, 90, 74),
        (54, 38, 26, 38, 28, 13, 90, 38),
        (104, 70, 114, 76, 12, 2, 121, 6),
        (139, 51, 133, 41, 12, 2, 149, -93),
        (109, 37, 100, 46, 13, 14, 45, 103),
        (129, 73, 137, 64, 12, 6, 42, 145),
    ]

    if len(lines) != 6:
        return False

    for i in range(6):
        actual = lines[i][0:]
        if not close_match(actual, expected[i]):
            return False

    return True

def unittest(data_path, temp_path):
    import image

    rgb = image.lab_to_rgb((74, -38, 30))
    return rgb[0] == 123 and rgb[1] == 199 and rgb[2] == 123

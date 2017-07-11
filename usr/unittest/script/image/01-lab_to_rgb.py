def unittest(data_path, temp_path):
    import image
    rgb = image.lab_to_rgb((76, -44, 34))
    return  (rgb[0] == 118 and rgb[1] == 207 and rgb[2] == 122)

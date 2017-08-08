def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/qrcode.pgm", copy_to_fb=True)
    codes = img.find_qrcodes()
    return len(codes) == 1 and codes[0][0:] == (76, 36, 168, 168, 'https://openmv.io', 1, 1, 3, 4, 0)

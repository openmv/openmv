def unittest(data_path, temp_path):
    import image
    img = image.Image("unittest/data/barcode.pgm", copy_to_fb=True)
    codes = img.find_barcodes()
    return len(codes) == 1 and codes[0][0:] == (61, 46, 514, 39, 'https://openmv.io/', 15, 0.0, 40)

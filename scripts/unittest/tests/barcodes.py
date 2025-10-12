def unittest(data_path, temp_path):
    import image

    img = image.Image(data_path + "/barcode.pgm", copy_to_fb=True)
    codes = img.find_barcodes()
    return codes[0][0:] == (11, 12, 514, 39, "https://openmv.io/", 15, 0.0, 40,)

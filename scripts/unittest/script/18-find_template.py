def unittest(data_path, temp_path):
    import image
    try:
        from image import SEARCH_EX, SEARCH_DS
    except Exception as e:
        raise Exception("function unavailable")
    img = image.Image("unittest/data/graffiti.pgm", copy_to_fb=True)
    temp = image.Image("unittest/data/template.pgm", copy_to_fb=False)
    r = img.find_template(temp, 0.70, step=4, search=SEARCH_DS)
    return r == (150, 128, 40, 40)

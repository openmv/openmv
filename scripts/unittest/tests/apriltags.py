def unittest(data_path, temp_path):
    import image

    img = image.Image(data_path + "/apriltags.pgm", copy_to_fb=True)
    tags = img.find_apriltags()

    if len(tags) != 1:
        return False

    return (
        tags[0].x == 45
        and tags[0].y == 27
        and tags[0].w == 69
        and tags[0].h == 69
        and tags[0].id == 255
        and tags[0].family == 16
        and tags[0].cx == 79
        and tags[0].cy == 61
    )

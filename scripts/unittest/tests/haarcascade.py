def unittest(data_path, temp_path):
    import image

    # Load Haar Cascade
    cascade = image.HaarCascade(data_path + "/frontalface.cascade")

    # Load image and find keypoints
    img = image.Image(data_path + "/dennis.pgm", copy_to_fb=True)

    # Find objects
    objects = img.find_features(cascade, threshold=0.75, scale_factor=1.25)
    return (
        objects and objects[0] == (189, 53, 88, 88) and objects[1] == (12, 11, 107, 107)
    )

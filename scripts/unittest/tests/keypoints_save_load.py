def unittest(data_path, temp_path):
    import image, os

    # Load image and find keypoints
    img = image.Image(data_path + "/graffiti.pgm", copy_to_fb=True)
    kpts1 = img.find_keypoints(max_keypoints=50, threshold=20, normalized=False)

    # Save descriptor
    image.save_descriptor(kpts1, temp_path + "/graffiti.orb")

    # Load descriptor
    kpts2 = image.load_descriptor(temp_path + "/graffiti.orb")

    # Match keypoints
    match = image.match_descriptor(kpts1, kpts2, threshold=85)

    return (
        match.cx() == 143
        and match.cy() == 108
        and match.x() == 36
        and match.y() == 34
        and match.w() == 251
        and match.h() == 141
        and match.count() == 50
        and match.theta() == 0
    )

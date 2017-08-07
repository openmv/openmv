def unittest(data_path, temp_path):
    import image, os
    # Load image and find keypoints
    img = image.Image(data_path+"/graffiti.pgm", copy_to_fb=True)
    kpts1 = img.find_keypoints(max_keypoints=150, threshold=20, normalized=False)

    # Save descriptor
    image.save_descriptor(kpts1, temp_path+"/graffiti2.orb")

    # Load descriptor
    kpts2 = image.load_descriptor(temp_path+"/graffiti2.orb")

    # Match keypoints
    match = image.match_descriptor(kpts1, kpts2, threshold=85)
    return  (match.cx()     == 138 and match.cy()     == 117 and \
             match.x()      == 36  and match.y()      == 34  and \
             match.w()      == 251 and match.h()      == 167 and \
             match.count()  == 150 and match.theta()  == 0)

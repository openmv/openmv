def unittest(data_path, temp_path):
    import csi

    # Initialize sensor with RGB565 format at QVGA resolution
    csi0 = csi.CSI()
    csi0.reset()
    csi0.pixformat(csi.RGB565)
    csi0.framesize(csi.QQVGA)

    # Capture a frame
    img = csi0.snapshot()
    #img.save(data_path + "/csi.ppm")
    stats_rgb = img.difference(data_path + "/csi.ppm").get_statistics()

    # Capture a frame
    csi0.reset()
    csi0.pixformat(csi.GRAYSCALE)
    csi0.framesize(csi.QQVGA)

    img = csi0.snapshot()
    #img.save(data_path + "/csi.pgm")
    stats_gs = img.difference(data_path + "/csi.pgm").get_statistics()

    # Compare with reference image
    return (stats_rgb.max() + stats_rgb.min() + stats_gs.max() + stats_gs.min()) == 0

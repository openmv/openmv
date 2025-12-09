def unittest(data_path, temp_path):
    try:
        import ml
    except ImportError:
        raise Exception("ML module unavailable")

    import image
    from ml.postprocessing.edgeimpulse import Fomo

    # Load image and run FOMO face detection model
    img = image.Image(data_path + "/faces.bmp", copy_to_fb=True)
    model = ml.Model("data/fomo_face_detection.tflite")
    output = model.predict([img], callback=Fomo())

    return output[1] == [([81, 107, 31, 31], 0.6796876), ([239, 153, 31, 31], 0.5742188), ([149, 17, 31, 31], 0.5390626), ([194, 198, 31, 31], 0.4804688), ([285, 17, 31, 31], 0.4257813)] 

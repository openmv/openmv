def unittest(data_path, temp_path):
    import ulinalg
    from vec import distance
    m = ulinalg.ones(1, 10)
    d = distance.euclidean((0, 0, 0), (0, 1, 0))
    return (abs(1.0-d) < 0.01) and (abs(10.0 - sum(m)) < 0.01)

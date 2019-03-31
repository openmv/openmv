def unittest(data_path, temp_path):
    import os, nn, image
    match = 0
    test_dir = data_path+"/cifar10"
    files = os.listdir(test_dir)
    total = len(files)
    labels = ['airplane', 'automobile', 'bird', 'cat', 'deer', 'dog', 'frog', 'horse', 'ship', 'truck']
    net = nn.load(data_path+'/cifar10_fast.network')
    for f in sorted(files):
        img = image.Image(test_dir+"/"+f, copy_to_fb=True)
        out = net.forward(img)
        label = labels[out.index(max(out))]
        if (f.split('-')[0] == label):
            match += 1
    return ((match / total * 100 ) > 90)

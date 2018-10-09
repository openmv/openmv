# CMSIS-NN

This folder contains scripts to train, test, and quantize Caffe models and then convert them to an 8-bit binary format for running on the OpenMV Cam.

## Getting started
1. Setup your computer for deep-learning:
    1. CPU Only - https://www.pyimagesearch.com/2017/09/25/configuring-ubuntu-for-deep-learning-with-python/
    2. GPU (recommended) - https://www.pyimagesearch.com/2017/09/27/setting-up-ubuntu-16-04-cuda-gpu-for-deep-learning-with-python/
2. Install caffe:
    1. `pushd ~` (in the folder this READMD.md is in)
    2. `git clone --recursive https://github.com/BVLC/caffe.git`
    3. Follow https://github.com/BVLC/caffe/wiki/Ubuntu-16.04-or-15.10-Installation-Guide
    4. Add `export PYTHONPATH=/home/<username>/caffe/:$PYTHONPATH` to your `~/.bashrc` file.
    5. `source ~/.bashrc`
    6. `popd`
    7. `ln -s ~/caffe caffe` (in the folder this READMD.md is in)
3. Read http://adilmoujahid.com/posts/2016/06/introduction-deep-learning-python-caffe/

## Training a CIFAR10 Model
1. Now we're going to train a CIFAR10 Model.
    1. Open a terminal in this folder.
    2. First we need to get the data and create an lmdb.
        1. `cd caffe`
        2. `./data/cifar10/get_cifar10.sh`
        3. `./examples/cifar10/create_cifar10.sh`
        4. `./build/tools/compute_image_mean -backend=lmdb examples/cifar10/cifar10_train_lmdb examples/cifar10/mean.binaryproto`
    3. Next we need to train our network.
        1. `cd ..`
        2. `./models/cifar10/train.sh` - This takes a while.
2. Great! Now let's test and then convert the network.
    1. `./models/cifar10/test.sh` - You should get an accuracy of about 80%.
    2. `python2 nn_quantizer.py --gpu --model models/cifar10/cifar10_train_test.prototxt --weights models/cifar10/cifar10_iter_70000.caffemodel.h5 --save models/cifar10/cifar10.pkl` - Note how the accuracy stays at about 80%.
    3. `python2 nn_convert.py --model models/cifar10/cifar10.pkl --mean caffe/examples/cifar10/mean.binaryproto --output models/cifar10/cifar10.network`.
    4. And that's it! You've created a CNN that will run on the OpenMV Cam! Keep in mind that your OpenMV Cam has limited weight/bias heap space so this limits the size of the network. To run the CNN copy the `models/cifar10/cifar10.network` file to your OpenMV Cam's disk and then run our CIFAR10 Machine Learning Examples.

## Training a CIFAR10 Fast Model
1. Now we're going to train a CIFAR10 Fast Model which is 60% smaller than the cifar10 network with only a 2% loss in accurary.
    1. Open a terminal in this folder.
    2. First we need to get the data and create an lmdb.
        1. `cd caffe`
        2. `./data/cifar10/get_cifar10.sh`
        3. `./examples/cifar10/create_cifar10.sh`
        4. `./build/tools/compute_image_mean -backend=lmdb examples/cifar10/cifar10_train_lmdb examples/cifar10/mean.binaryproto`
    3. Next we need to train our network.
        1. `cd ..`
        2. `./models/cifar10_fast/train.sh` - This takes a while.
2. Great! Now let's test and then convert the network.
    1. `./models/cifar10_fast/test.sh` - You should get an accuracy of about 78%.
    2. `python2 nn_quantizer.py --gpu --model models/cifar10_fast/cifar10_fast_train_test.prototxt --weights models/cifar10_fast/cifar10_fast_iter_70000.caffemodel.h5 --save models/cifar10_fast/cifar10_fast.pkl` - Note how the accuracy stays at about 78%.
    3. `python2 nn_convert.py --model models/cifar10_fast/cifar10_fast.pkl --mean caffe/examples/cifar10/mean.binaryproto --output models/cifar10_fast/cifar10_fast.network`.
    4. And that's it! You've created a CNN that will run on the OpenMV Cam! Keep in mind that your OpenMV Cam has limited weight/bias heap space so this limits the size of the network. To run the CNN copy the `models/cifar10_fast/cifar10_fast.network` file to your OpenMV Cam's disk and then run our CIFAR10 Machine Learning Examples using the cifar10_fast.network.

## Training a MNIST Model
1. Now we're going to train a MNIST Model.
    1. Open a terminal in this folder.
    2. First we need to get the data and create an lmdb.
        1. `cd caffe`
        2. `./data/mnist/get_mnist.sh`
        3. `./examples/mnist/create_mnist.sh`
        4. `./build/tools/compute_image_mean -backend=lmdb examples/mnist/mnist_train_lmdb examples/mnist/mean.binaryproto`
    3. Next we need to train our network.
        1. `cd ..`
        2. `./models/lenet/train.sh` - This takes a while.
2. Great! Now let's test and then convert the network.
    1. `./models/lenet/test.sh` - You should get an accuracy of about 99%.
    2. `python2 nn_quantizer.py --gpu --model models/lenet/lenet_train_test.prototxt --weights models/lenet/lenet_iter_10000.caffemodel --save models/lenet/lenet.pkl` - Note how the accuracy stays at about 99%.
    3. `python2 nn_convert.py --model models/lenet/lenet.pkl --mean caffe/examples/mnist/mean.binaryproto --output models/lenet/lenet.network`.
    4. And that's it! You've created a CNN that will run on the OpenMV Cam! Keep in mind that your OpenMV Cam has limited weight/bias heap space so this limits the size of the network. To run the CNN copy the `models/lenet/lenet.network` file to your OpenMV Cam's disk and then run our LENET Machine Learning Examples.

## Training a Smile Detection Model
1. Now we're going to train a Smile Detection Model.
    1. Open a terminal in this folder.
    2. First we need to get the data and create an lmdb.
        1. `cd caffe/examples`
        2. `mkdir smile`
        3. `cd smile`
        4. `git clone --recursive https://github.com/hromi/SMILEsmileD.git`
        5. `cd ../../../../..`
        6. The SMILEsmileD dataset has about ~3K positive images and ~9K negative images so we need to augment our positive image dataset so that it is about the same size as our negative image dataset.
            1. `mkdir ml/cmsisnn/caffe/examples/smile/data`
            2. `cp -r ml/cmsisnn/caffe/examples/smile/SMILEsmileD/SMILEs/negatives/negatives7/ ml/cmsisnn/caffe/examples/smile/data/1_negatives`
            3. `sudo pip2 install opencv-python imgaug tqdm`
            4. `mkdir ml/cmsisnn/caffe/examples/smile/data/0_positives`
            5. `python2 tools/augment_images.py --input ml/cmsisnn/caffe/examples/smile/SMILEsmileD/SMILEs/positives/positives7/ --output ml/cmsisnn/caffe/examples/smile/data/0_positives/ --count 3`
        7. Now we need to create an lmdb.
            1. `mkdir ml/cmsisnn/caffe/examples/smile/lmdbin`
            2. `python2 tools/create_labels.py --input ml/cmsisnn/caffe/examples/smile/data/ --output ml/cmsisnn/caffe/examples/smile/lmdbin/`
            3. `cd ml/cmsisnn/caffe/`
            4. `GLOG_logtostderr=1 ./build/tools/convert_imageset --shuffle examples/smile/lmdbin/ examples/smile/train.txt examples/smile/train_lmdb`
            5. `GLOG_logtostderr=1 ./build/tools/convert_imageset --shuffle examples/smile/lmdbin/ examples/smile/test.txt examples/smile/test_lmdb`
        8. `./build/tools/compute_image_mean -backend=lmdb examples/smile/train_lmdb examples/smile/mean.binaryproto`
    3. Next we need to train our network.
        1. `cd ..`
        2. `./models/smile/train.sh` - This takes a while.
2. Great! Now let's test and then convert the network.
    1. `./models/smile/test.sh` - You should get an accuracy of about 96%.
    2. `python2 nn_quantizer.py --gpu --model models/smile/smile_train_test.prototxt --weights models/smile/smile_iter_200000.caffemodel --save models/smile/smile.pkl` - Note how the accuracy stays at about 96%.
    3. `python2 nn_convert.py --model models/smile/smile.pkl --mean caffe/examples/smile/mean.binaryproto --output models/smile/smile.network`.
    4. And that's it! You've created a CNN that will run on the OpenMV Cam! Keep in mind that your OpenMV Cam has limited weight/bias heap space so this limits the size of the network. To run the CNN copy the `models/smile/smile.network` file to your OpenMV Cam's disk and then run our Smile Machine Learning Example.

### Train A Custom Net
If you'd like to train your own custom CNN you need to assemble a dataset of hundreds (preferably thousands) of images of training examples. Once you've collected all the training examples save the images per class of training examples in seperate folders structed like this:
* data/
    * 0_some_class/
    * 1_some_other_class/
    * 2_etc/

Once you've built a folder structure like this please refer to the examples above to:
1. Create a labeled training dataset using augment_images.py and create_labels.py.
2. Create training and test lmdb files.
3. Create a mean.binaryproto file.
4. Train the network (copy how the smile train.sh script and solver/train/test protobufs work to do this).
5. Test the network (copy how the smile test.sh script and solver/train/test protobufs works to do this).
6. Quantize the network.
7. And finally convert the network.

### Known Limitations 
1. Parser supports conv, pool, relu, fc layers only.
2. Quantizer supports only networks with feed-forward structures (e.g. conv-relu-pool-fc)  without branch-out/branch-in (as in inception/squeezeNet, etc.).
3. See [ARM ML-examples](https://github.com/ARM-software/ML-examples/tree/master/cmsisnn-cifar10) for more information.
4. Source Code https://github.com/ARM-software/CMSIS_5/tree/develop/CMSIS/NN

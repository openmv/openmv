# CMSIS-NN Models and Scripts.

This folder contains scripts to quantize trained Caffe models to 8-bits and convert them to binary format for the OpenMV camera. See [ARM ML-examples](https://github.com/ARM-software/ML-examples/tree/master/cmsisnn-cifar10)

## Getting started
1. Make sure caffe is installed and it's python path is added in $PYTHONPATH environment variable.
2. Add a symlink to caffe root directory to cmsisnn directory:
```bash
ln -s /path/to/caffe/ caffe
```

## Usage
1. Run *nn_quantizer.py* to parse and quantize the network.
```bash
python2 nn_quantizer.py --model models/cifar10/cifar10_train_test.prototxt \
  --weights models/cifar10/cifar10_iter_300000.caffemodel.h5 \
  --save models/cifar10/cifar10.pkl
```

2. Convert to binary format to run on OpenMV camera.
```bash
python2 nn_convert.py --model models/cifar10/cifar10.pkl \
  --mean models/cifar10/mean.binaryproto \
  --output models/cifar10/cifar10.network
```

### Common Problems 
1. `ImportError: No module named caffe`
Add Caffe python installation path to $PYTHONPATH environment variable, e.g., `export PYTHONPATH="/home/ubuntu_user/caffe/python:$PYTHONPATH"`
2. `F0906 15:49:48.701362 11933 db_lmdb.hpp:15] Check failed: mdb_status == 0 (2 vs. 0) No such file or directory`
Make sure valid dataset (lmdb) is present in the model prototxt definition, as the dataset is required to find the quantization ranges for activations.

### Known Limitations 
1. Parser supports conv, pool, relu, fc layers only.
2. Quantizer supports only networks with feed-forward structures (e.g. conv-relu-pool-fc)  without branch-out/branch-in (as in inception/squeezeNet, etc.).

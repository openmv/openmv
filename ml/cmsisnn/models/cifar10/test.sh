#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe test \
    --model=models/cifar10/cifar10_train_test.prototxt \
    --weights=models/cifar10/cifar10_iter_70000.caffemodel.h5 $@

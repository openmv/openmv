#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe test \
    --model=models/cifar10_fast/cifar10_fast_train_test.prototxt \
    --weights=models/cifar10_fast/cifar10_fast_iter_70000.caffemodel.h5 $@

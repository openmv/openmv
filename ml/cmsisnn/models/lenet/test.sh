#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe test \
    --model=models/lenet/lenet_train_test.prototxt \
    --weights=models/lenet/lenet_iter_10000.caffemodel $@

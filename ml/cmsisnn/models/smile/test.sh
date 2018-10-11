#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe test \
    --model=models/smile/smile_train_test.prototxt \
    --weights=models/smile/smile_iter_200000.caffemodel $@

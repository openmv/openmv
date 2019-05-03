#!/usr/bin/env sh
set -e

DIR=models/cifar10_fast
TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/cifar10_fast/cifar10_fast_solver.prototxt $@ 2>&1 | tee $DIR/training.log

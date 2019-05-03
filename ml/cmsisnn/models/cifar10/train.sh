#!/usr/bin/env sh
set -e

DIR=models/cifar10
TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/cifar10/cifar10_solver.prototxt $@ 2>&1 | tee $DIR/training.log

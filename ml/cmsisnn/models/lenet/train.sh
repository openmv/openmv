#!/usr/bin/env sh
set -e

DIR=models/lenet
TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/lenet/lenet_solver.prototxt $@ 2>&1 | tee $DIR/training.log

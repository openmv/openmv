#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/lenet/lenet_solver.prototxt $@

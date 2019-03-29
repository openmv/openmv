#!/usr/bin/env sh
set -e

DIR=models/smile
TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/smile/smile_solver.prototxt $@  2>&1 | tee $DIR/training.log

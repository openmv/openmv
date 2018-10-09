#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/smile/smile_solver.prototxt $@

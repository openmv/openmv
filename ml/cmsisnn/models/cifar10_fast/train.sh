#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/cifar10_fast/cifar10_fast_solver.prototxt $@

# reduce learning rate by factor of 10
$TOOLS/caffe train \
    --solver=models/cifar10_fast/cifar10_fast_solver_lr1.prototxt \
    --snapshot=models/cifar10_fast/cifar10_fast_iter_60000.solverstate.h5 $@

# reduce learning rate by factor of 10
$TOOLS/caffe train \
    --solver=models/cifar10_fast/cifar10_fast_solver_lr2.prototxt \
    --snapshot=models/cifar10_fast/cifar10_fast_iter_65000.solverstate.h5 $@

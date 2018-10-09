#!/usr/bin/env sh
set -e

TOOLS=./caffe/build/tools

$TOOLS/caffe train \
    --solver=models/cifar10/cifar10_solver.prototxt $@

# reduce learning rate by factor of 10
$TOOLS/caffe train \
    --solver=models/cifar10/cifar10_solver_lr1.prototxt \
    --snapshot=models/cifar10/cifar10_iter_60000.solverstate.h5 $@

# reduce learning rate by factor of 10
$TOOLS/caffe train \
    --solver=models/cifar10/cifar10_solver_lr2.prototxt \
    --snapshot=models/cifar10/cifar10_iter_65000.solverstate.h5 $@

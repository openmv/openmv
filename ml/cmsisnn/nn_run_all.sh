#!/usr/bin/env sh
# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2019 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2019 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.

if [ -z $1 ]; then
    echo "Usage : nn_run_all.sh model_name"
    exit 1
fi
MODEL=${1}
set -v
python2 nn_quantizer.py --model models/${MODEL}/${MODEL}_train_test.prototxt --weights models/${MODEL}/${MODEL}_iter_*.caffemodel --save models/${MODEL}/${MODEL}.pkl --gpu 
python2 nn_convert.py   --model models/${MODEL}/${MODEL}.pkl --mean caffe/examples/${MODEL}/mean.binaryproto --output models/${MODEL}/${MODEL}.network

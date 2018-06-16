#!/usr/bin/env sh
if [ -z $1 ]; then
    echo "Usage : nn_run_all.sh model_name"
    exit 1
fi
MODEL=${1}
set -v
python2 nn_quantizer.py --model models/${MODEL}/${MODEL}_train_test.prototxt --weights models/${MODEL}/${MODEL}_iter_*.caffemodel --save models/${MODEL}/${MODEL}.pkl --gpu 
python2 nn_convert.py   --model models/${MODEL}/${MODEL}.pkl --mean caffe/examples/${MODEL}/mean.binaryproto --output models/${MODEL}/${MODEL}.network

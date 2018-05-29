# This file is part of the OpenMV project.
# Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# CMSIS NN binary converter.

import numpy as np
import pickle, struct
import os, sys, caffe, argparse
from nn_quantizer import *
from caffe.proto import caffe_pb2
from google.protobuf import text_format

caffe_layers = {
    'data'          : 0,
    'convolution'   : 1,
    'relu'          : 2,
    'pooling'       : 3,
    'innerproduct'  : 4
}

def get_mean_values(mean_file):
    mean_vals = [0, 0, 0]
    if (mean_file):
        with open(mean_file, 'rb') as f:
            data = f.read()
        blob = caffe.proto.caffe_pb2.BlobProto()
        blob.ParseFromString(data)
        arr = np.array(caffe.io.blobproto_to_array(blob))[0]
        mean_vals = [int(x.mean().round()) for x in arr]
    return mean_vals

def convert_to_x4_weights(weights):
    """This function convert the fully-connected layer weights
       to the format that accepted by X4 implementation"""
    [r, h, w, c] = weights.shape
    weights = np.reshape(weights, (r, h*w*c))
    num_of_rows = r
    num_of_cols = h*w*c
    new_weights = np.copy(weights)
    new_weights = np.reshape(new_weights, (r*h*w*c))
    counter = 0
    for i in range(int(num_of_rows)/4):
      # we only need to do the re-ordering for every 4 rows
      row_base = 4*i
      for j in range (int(num_of_cols)/4):
        # for each 4 entries
        column_base = 4*j
        new_weights[counter]   =  weights[row_base  ][column_base  ]
        new_weights[counter+1] =  weights[row_base+1][column_base  ]
        new_weights[counter+2] =  weights[row_base  ][column_base+2]
        new_weights[counter+3] =  weights[row_base+1][column_base+2]
        new_weights[counter+4] =  weights[row_base+2][column_base  ]
        new_weights[counter+5] =  weights[row_base+3][column_base  ]
        new_weights[counter+6] =  weights[row_base+2][column_base+2]
        new_weights[counter+7] =  weights[row_base+3][column_base+2]

        new_weights[counter+8] =  weights[row_base  ][column_base+1]
        new_weights[counter+9] =  weights[row_base+1][column_base+1]
        new_weights[counter+10] = weights[row_base  ][column_base+3]
        new_weights[counter+11] = weights[row_base+1][column_base+3]
        new_weights[counter+12] = weights[row_base+2][column_base+1]
        new_weights[counter+13] = weights[row_base+3][column_base+1]
        new_weights[counter+14] = weights[row_base+2][column_base+3]
        new_weights[counter+15] = weights[row_base+3][column_base+3]
        counter = counter + 16
      # the remaining ones are in order
      for j in range((int)(num_of_cols-num_of_cols%4), int(num_of_cols)):
        new_weights[counter] = weights[row_base][j]
        new_weights[counter+1] = weights[row_base+1][j]
        new_weights[counter+2] = weights[row_base+2][j]
        new_weights[counter+3] = weights[row_base+3][j]
        counter = counter + 4
    return new_weights

def dump_network(caffe_model, file_name):
    fout = open(file_name, 'wb')
    net = caffe.Net(caffe_model.model_file, caffe_model.quant_weight_file, caffe.TEST)

    # Write network type
    fout.write(struct.pack('4c', 'C', 'A', 'F', 'E'))

    num_layers = 0
    # Check and count layers
    for layer in caffe_model.layer:
        layer_type = caffe_model.layer_type[layer]
        if layer_type in caffe_layers:
            num_layers += 1
        elif layer_type != 'accuracy':
            print("Layer %s is not supported, can't convert this network."%(layer_type))
            sys.exit(1)

    # Write number of layers
    fout.write(struct.pack('i', num_layers))

    for layer in caffe_model.layer:
        layer_no = caffe_model.layer.index(layer)
        layer_type = caffe_model.layer_type[layer]

        if not layer_type in caffe_layers:
            print('NOTE: skipping layer "%s"' %(layer_type))
            continue

        if layer_no > 0:
            prev_layer = caffe_model.layer[layer_no-1]

        # Write layer type code
        fout.write(struct.pack('i', caffe_layers[layer_type]))

        # Write layer shape (n, c, h, w)
        shape = [x for x in caffe_model.layer_shape[layer]]
        if (len(shape) < 4): shape += [1] * (4 - len(shape))
        fout.write(struct.pack('4i', *shape))
        caffe_model.layer_shape[layer] = shape

        print('Layer: {0: <8} Type: {1: <15}Shape: {2: <20}'.format(layer, layer_type, str(shape)))

        if layer_type == 'data':
            # Write r_mean, g_mean, b_mean
            mean_values = get_mean_values(model.mean_file)
            fout.write(struct.pack('3i', *mean_values))
            # Write input scale
            fout.write(struct.pack('i', 8-caffe_model.act_dec_bits[layer]))

        if layer_type == 'pooling':
            # Write pool type
            fout.write(struct.pack('i', caffe_model.pool_type[layer]))

        if layer_type in ['convolution', 'innerproduct']:
            # Write lshift, rshift
            fout.write(struct.pack('i', max(0, caffe_model.bias_lshift[layer])))
            fout.write(struct.pack('i', max(0, caffe_model.act_rshift[layer])))

        if layer_type in ['convolution', 'pooling']:
            # Write k_size, k_pad, k_stride
            fout.write(struct.pack('i', caffe_model.kernel_size[layer]))
            fout.write(struct.pack('i', caffe_model.pad[layer]))
            fout.write(struct.pack('i', caffe_model.stride[layer]))

        if layer_type == 'convolution':
            net.params[layer][0].data[:] = np.round(net.params[layer][0].data*(2**caffe_model.wt_dec_bits[layer]))
            net.params[layer][1].data[:] = np.round(net.params[layer][1].data*(2**caffe_model.bias_dec_bits[layer]))

            #CHW to HWC layout conversion
            reordered_wts = np.swapaxes(np.swapaxes(net.params[layer][0].data, 1, 2), 2, 3).flatten()

            # Write weights size and array
            fout.write(struct.pack('i', len(reordered_wts)))
            for i in reordered_wts: fout.write(struct.pack('b', i))

            # Write bias size and array
            fout.write(struct.pack('i', len(net.params[layer][1].data)))
            for i in net.params[layer][1].data: fout.write(struct.pack('b', i))

        if layer_type  == 'innerproduct':
            net.params[layer][0].data[:]=np.round(net.params[layer][0].data*(2**caffe_model.wt_dec_bits[layer]))
            net.params[layer][1].data[:]=np.round(net.params[layer][1].data*(2**caffe_model.bias_dec_bits[layer]))
            layer_no = caffe_model.layer.index(layer)
            prev_layer_name = caffe_model.layer[layer_no-1]  #needed to find input shape of 'ip' layer
            if(len(caffe_model.layer_shape[prev_layer_name])>2): #assuming HWC input format
                reshaped_shape = (caffe_model.layer_shape[layer][1],caffe_model.layer_shape[prev_layer_name][1],\
                    caffe_model.layer_shape[prev_layer_name][2],caffe_model.layer_shape[prev_layer_name][3])
                reordered_wts = np.reshape(net.params[layer][0].data, reshaped_shape)
                # Reorder the weights to use fully_connected_x4 kernel
                reordered_wts = np.swapaxes(np.swapaxes(reordered_wts, 1, 2), 2, 3)
                reordered_wts = convert_to_x4_weights(reordered_wts)
            else:
                reordered_wts = net.params[layer][0].data.flatten()

            # Write weights size and array
            fout.write(struct.pack('i', len(reordered_wts)))
            for i in reordered_wts: fout.write(struct.pack('b', i))

            # Write bias size and array
            fout.write(struct.pack('i', len(net.params[layer][1].data)))
            for i in net.params[layer][1].data: fout.write(struct.pack('b', i))

    fout.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', type=str, help='model info')
    parser.add_argument('--mean',  type=str, help='mean image file')
    parser.add_argument('--output',type=str, default="cifar10.network", help='output file')

    args, _ = parser.parse_known_args()

    model = Caffe_Quantizer()
    model.load_quant_params(args.model)
    model.mean_file = args.mean if (args.mean) else None
    dump_network(model, args.output)

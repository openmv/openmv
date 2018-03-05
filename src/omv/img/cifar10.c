/* ----------------------------------------------------------------------
* Copyright (C) 2010-2018 Arm Limited. All rights reserved.
*
* Project:       CMSIS NN Library
* Description:   Convolutional Neural Network Example
* Target Processor: Cortex-M4/Cortex-M7
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*   - Neither the name of Arm LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
* -------------------------------------------------------------------- */

/**
 * Convolutional Neural Network Example
 *
 * Description:
 * Demonstrates a convolutional neural network (CNN) example with the use of convolution,
 * ReLU activation, pooling and fully-connected functions.
 *
 * Model definition:
 * The CNN used in this example is based on CIFAR-10 example from Caffe [1]. The neural network
 * consists of 3 convolution layers interspersed by ReLU activation and max pooling layers, followed
 * by a fully-connected layer at the end. The input to the network is a 32x32 pixel color image,
 * which will be classified into one of the 10 output classes. 
 * This example model implementation needs 32.3 KB to store weights, 40 KB for activations and 
 * 3.1 KB for storing the im2col data.
 *
 * Neural Network model definition:
 * Variables Description:
 * conv1_wt, conv2_wt, conv3_wt are convolution layer weight matrices
 * conv1_bias, conv2_bias, conv3_bias are convolution layer bias arrays
 * ip1_wt, ip1_bias point to fully-connected layer weights and biases
 * input_data points to the input image data
 * output_data points to the classification output
 * col_buffer is a buffer to store the im2col output
 * scratch_buffer is used to store the activation data (intermediate layer outputs)
 *
 * CMSIS DSP Software Library Functions Used:
 * - arm_convolve_HWC_q7_RGB()
 * - arm_convolve_HWC_q7_fast()
 * - arm_relu_q7()
 * - arm_maxpool_q7_HWC()
 * - arm_avepool_q7_HWC()
 * - arm_fully_connected_q7_opt()
 * - arm_fully_connected_q7()
 *
 * [1] https://github.com/BVLC/caffe
 */
#include <stdint.h>
#include <stdio.h>
#include "imlib.h"
#include "fb_alloc.h"
#include "arm_math.h"
#include "arm_nnfunctions.h"

#ifdef IMLIB_ENABLE_CNN
#define CONV1_IM_DIM        (32)
#define CONV1_IM_CH         (3)
#define CONV1_KER_DIM       (5)
#define CONV1_PADDING       (2)
#define CONV1_STRIDE        (1)
#define CONV1_OUT_CH        (32)
#define CONV1_OUT_DIM       (32)

#define POOL1_KER_DIM       (3)
#define POOL1_STRIDE        (2)
#define POOL1_PADDING       (0)
#define POOL1_OUT_DIM       (16)

#define CONV2_IM_DIM        (16)
#define CONV2_IM_CH         (32)
#define CONV2_KER_DIM       (5)
#define CONV2_PADDING       (2)
#define CONV2_STRIDE        (1)
#define CONV2_OUT_CH        (16)
#define CONV2_OUT_DIM       (16)

#define POOL2_KER_DIM       (3)
#define POOL2_STRIDE        (2)
#define POOL2_PADDING       (0)
#define POOL2_OUT_DIM       (8)

#define CONV3_IM_DIM        (8)
#define CONV3_IM_CH         (16)
#define CONV3_KER_DIM       (5)
#define CONV3_PADDING       (2)
#define CONV3_STRIDE        (1)
#define CONV3_OUT_CH        (32)
#define CONV3_OUT_DIM       (8)

#define POOL3_KER_DIM       (3)
#define POOL3_STRIDE        (2)
#define POOL3_PADDING       (0)
#define POOL3_OUT_DIM       (4)

#define IP1_DIM             (4*4*32)
#define IP1_IM_DIM          (4)
#define IP1_IM_CH           (32)
#define IP1_OUT             (10)

#define CONV1_BIAS_LSHIFT   (0)
#define CONV1_OUT_RSHIFT    (11)
#define CONV2_BIAS_LSHIFT   (0)
#define CONV2_OUT_RSHIFT    (8)
#define CONV3_BIAS_LSHIFT   (0)
#define CONV3_OUT_RSHIFT    (8)
#define IP1_BIAS_LSHIFT     (5)
#define IP1_OUT_RSHIFT      (7)

// include the input and weights
extern const q7_t cifar10_mean_image[CONV1_IM_CH * CONV1_IM_DIM * CONV1_IM_DIM];
extern const q7_t cifar10_conv1_wt[CONV1_IM_CH * CONV1_KER_DIM * CONV1_KER_DIM * CONV1_OUT_CH];
extern const q7_t cifar10_conv1_bias[CONV1_OUT_CH];

extern const q7_t cifar10_conv2_wt[CONV2_IM_CH * CONV2_KER_DIM * CONV2_KER_DIM * CONV2_OUT_CH];
extern const q7_t cifar10_conv2_bias[CONV2_OUT_CH];

extern const q7_t cifar10_conv3_wt[CONV3_IM_CH * CONV3_KER_DIM * CONV3_KER_DIM * CONV3_OUT_CH];
extern const q7_t cifar10_conv3_bias[CONV3_OUT_CH];

extern const q7_t cifar10_ip1_wt[IP1_DIM * IP1_OUT];
extern const q7_t cifar10_ip1_bias[IP1_OUT];

int imlib_classify_object(image_t *img, int8_t *output_data)
{
    //TODO: Load from file.
    const q7_t *mean_image  = cifar10_mean_image;
    const q7_t *conv1_wt    = cifar10_conv1_wt;
    const q7_t *conv1_bias  = cifar10_conv1_bias;
                                               
    const q7_t *conv2_wt    = cifar10_conv2_wt;
    const q7_t *conv2_bias  = cifar10_conv2_bias;
                                               
    const q7_t *conv3_wt    = cifar10_conv3_wt;
    const q7_t *conv3_bias  = cifar10_conv3_bias;
                                               
    const q7_t *ip1_wt      = cifar10_ip1_wt;
    const q7_t *ip1_bias    = cifar10_ip1_bias;

    q7_t *input_data  = fb_alloc0(CONV1_IM_CH * CONV1_IM_DIM * CONV1_IM_DIM);
    q7_t *col_buffer  = fb_alloc0(2 * 5 * 5 * 32 * 2);
    q7_t *img_buffer1 = fb_alloc0(32 * 32 * 10 * 4);
    q7_t *img_buffer2 = img_buffer1 + 32 * 32 * 32;

    // Scale, convert, remove mean image and load input data.
    int x_ratio = (int)((img->w<<16)/32)+1;
    int y_ratio = (int)((img->h<<16)/32)+1;
	for (int y=0, i=0; y<32; y++) {
        int sy = (y*y_ratio)>>16;
		for (int x=0; x<32; x++, i+=3) {
            int sx = (x*x_ratio)>>16;
            uint16_t p = IM_GET_RGB565_PIXEL(img, sx, sy);
            input_data[i+0] = (int8_t) (((int) COLOR_RGB565_TO_R8(p)) - (int) mean_image[i+0]);
            input_data[i+1] = (int8_t) (((int) COLOR_RGB565_TO_G8(p)) - (int) mean_image[i+1]);
            input_data[i+2] = (int8_t) (((int) COLOR_RGB565_TO_B8(p)) - (int) mean_image[i+2]);
        }
    }

    // conv1 input_data -> img_buffer1
    arm_convolve_HWC_q7_RGB(input_data, CONV1_IM_DIM, CONV1_IM_CH, conv1_wt, CONV1_OUT_CH, CONV1_KER_DIM, CONV1_PADDING,
            CONV1_STRIDE, conv1_bias, CONV1_BIAS_LSHIFT, CONV1_OUT_RSHIFT, img_buffer1, CONV1_OUT_DIM,
            (q15_t *) col_buffer, NULL);

    arm_relu_q7(img_buffer1, CONV1_OUT_DIM * CONV1_OUT_DIM * CONV1_OUT_CH);

    // pool1 img_buffer1 -> img_buffer2
    arm_maxpool_q7_HWC(img_buffer1, CONV1_OUT_DIM, CONV1_OUT_CH, POOL1_KER_DIM,
            POOL1_PADDING, POOL1_STRIDE, POOL1_OUT_DIM, NULL, img_buffer2);

    // conv2 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV2_IM_DIM, CONV2_IM_CH, conv2_wt, CONV2_OUT_CH, CONV2_KER_DIM,
            CONV2_PADDING, CONV2_STRIDE, conv2_bias, CONV2_BIAS_LSHIFT, CONV2_OUT_RSHIFT, img_buffer1,
            CONV2_OUT_DIM, (q15_t *) col_buffer, NULL);

    arm_relu_q7(img_buffer1, CONV2_OUT_DIM * CONV2_OUT_DIM * CONV2_OUT_CH);

    // pool2 img_buffer1 -> img_buffer2
    arm_avepool_q7_HWC(img_buffer1, CONV2_OUT_DIM, CONV2_OUT_CH, POOL2_KER_DIM,
            POOL2_PADDING, POOL2_STRIDE, POOL2_OUT_DIM, col_buffer, img_buffer2);

    // conv3 img_buffer2 -> img_buffer1
    arm_convolve_HWC_q7_fast(img_buffer2, CONV3_IM_DIM, CONV3_IM_CH, conv3_wt, CONV3_OUT_CH, CONV3_KER_DIM,
            CONV3_PADDING, CONV3_STRIDE, conv3_bias, CONV3_BIAS_LSHIFT, CONV3_OUT_RSHIFT, img_buffer1,
            CONV3_OUT_DIM, (q15_t *) col_buffer, NULL);

    arm_relu_q7(img_buffer1, CONV3_OUT_DIM * CONV3_OUT_DIM * CONV3_OUT_CH);

    // pool3 img_buffer-> img_buffer2
    arm_avepool_q7_HWC(img_buffer1, CONV3_OUT_DIM, CONV3_OUT_CH, POOL3_KER_DIM,
            POOL3_PADDING, POOL3_STRIDE, POOL3_OUT_DIM, col_buffer, img_buffer2);

    #if 1
    arm_fully_connected_q7_opt(img_buffer2, ip1_wt, IP1_DIM, IP1_OUT, IP1_BIAS_LSHIFT, IP1_OUT_RSHIFT, ip1_bias,
            output_data, (q15_t *) img_buffer1);
    #else
    arm_fully_connected_q7(img_buffer2, ip1_wt, IP1_DIM, IP1_OUT, IP1_BIAS_LSHIFT, IP1_OUT_RSHIFT, ip1_bias,
            output_data, (q15_t *) img_buffer1);
    #endif

    arm_softmax_q7(output_data, 10, output_data);

    fb_free_all();
    return 0;
}

#endif //IMLIB_ENABLE_CNN

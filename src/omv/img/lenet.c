/* This file is part of the OpenMV project.
 * Copyright (c) 2013-2017 Ibrahim Abdelkader <iabdalkader@openmv.io> & Kwabena W. Agyeman <kwagyeman@openmv.io>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */

/*
 * 范文捷
 * 2016-04-20
 * 根据Yann Lecun的论文《Gradient-based Learning Applied To Document Recognition》编写
 */

#include "imlib.h"
#include "fb_alloc.h"

#define GETLENGTH(array) (sizeof(array)/sizeof(*(array)))

#define GETCOUNT(array)  (sizeof(array)/sizeof(float))

#define FOREACH(i,count) for (int i = 0; i < count; ++i)

#define CONVOLUTE_VALID(input,output,weight)											\
{																						\
	FOREACH(o0,GETLENGTH(output))														\
		FOREACH(o1,GETLENGTH(*(output)))												\
			FOREACH(w0,GETLENGTH(weight))												\
				FOREACH(w1,GETLENGTH(*(weight)))										\
					(output)[o0][o1] += (input)[o0 + w0][o1 + w1] * (weight)[w0][w1];	\
}

#define CONVOLUTION_FORWARD(input,output,weight,bias,action)					\
{																				\
	for (int x = 0; x < GETLENGTH(weight); ++x)									\
		for (int y = 0; y < GETLENGTH(*weight); ++y)							\
			CONVOLUTE_VALID(input[x], output[y], weight[x][y]);					\
	FOREACH(j, GETLENGTH(output))												\
		FOREACH(i, GETCOUNT(output[j]))											\
		((float *)output[j])[i] = action(((float *)output[j])[i] + bias[j]);	\
}

#define SUBSAMP_MAX_FORWARD(input,output)														\
{																								\
	const int len0 = GETLENGTH(*(input)) / GETLENGTH(*(output));								\
	const int len1 = GETLENGTH(**(input)) / GETLENGTH(**(output));								\
	FOREACH(i, GETLENGTH(output))																\
	FOREACH(o0, GETLENGTH(*(output)))															\
	FOREACH(o1, GETLENGTH(**(output)))															\
	{																							\
		int x0 = 0, x1 = 0, ismax;																\
		FOREACH(l0, len0)																		\
			FOREACH(l1, len1)																	\
		{																						\
			ismax = input[i][o0*len0 + l0][o1*len1 + l1] > input[i][o0*len0 + x0][o1*len1 + x1];\
			x0 += ismax * (l0 - x0);															\
			x1 += ismax * (l1 - x1);															\
		}																						\
		output[i][o0][o1] = input[i][o0*len0 + x0][o1*len1 + x1];								\
	}																							\
}

#define DOT_PRODUCT_FORWARD(input,output,weight,bias,action)				\
{																			\
	for (int x = 0; x < GETLENGTH(weight); ++x)								\
		for (int y = 0; y < GETLENGTH(*weight); ++y)						\
			((float *)output)[y] += ((float *)input)[x] * weight[x][y];	\
	FOREACH(j, GETLENGTH(bias))												\
		((float *)output)[j] = action(((float *)output)[j] + bias[j]);	\
}

float relu(float x)
{
    return x*(x > 0);
}

float relugrad(float y)
{
    return y > 0;
}

static void forward(lenet5_t *lenet, lenet5_feature_t *features, float(*action)(float))
{
    CONVOLUTION_FORWARD(features->input, features->layer1, lenet->weight0_1, lenet->bias0_1, action);
    SUBSAMP_MAX_FORWARD(features->layer1, features->layer2);
    CONVOLUTION_FORWARD(features->layer2, features->layer3, lenet->weight2_3, lenet->bias2_3, action);
    SUBSAMP_MAX_FORWARD(features->layer3, features->layer4);
    CONVOLUTION_FORWARD(features->layer4, features->layer5, lenet->weight4_5, lenet->bias4_5, action);
    DOT_PRODUCT_FORWARD(features->layer5, features->output, lenet->weight5_6, lenet->bias5_6, action);
}

static inline void load_input(lenet5_feature_t *features, image_t *src, rectangle_t *r)
{
    float mean = 0, std = 0;
    float (*layer0)[LENGTH_FEATURE0][LENGTH_FEATURE0] = features->input;
    for (int k=r->y; k<r->y+r->h; k++) {
        for (int j=r->x; j<r->x+r->w; j++) {
            uint8_t p = imlib_get_pixel(src, j, k);
            mean += p;
            std  += p * p;
        }
    }
    mean /= (r->w*r->h);
    std = fast_sqrtf(std / (r->w*r->h) - mean*mean);
    for (int k=r->y; k<r->y+r->h; k++) {
        for (int j=r->x; j<r->x+r->w; j++) {
            layer0[0][(k-r->y) + LENET_PADDING_SIZE][(j-r->x)+ LENET_PADDING_SIZE] = (imlib_get_pixel(src, j, k) - mean) / std;
        }
    }
}

static inline void softmax(float *input, float *loss, int label, int count)
{
    float inner = 0;
    for (int i = 0; i < count; ++i)
    {
        float res = 0;
        for (int j = 0; j < count; ++j)
        {
            res += expf(input[j] - input[i]);
        }
        loss[i] = 1. / res;
        inner -= loss[i] * loss[i];
    }
    inner += loss[label];
    for (int i = 0; i < count; ++i)
    {
        loss[i] *= (i == label) - loss[i] - inner;
    }
}

uint8_t lenet_predict(lenet5_t *lenet, image_t *src, rectangle_t *roi, float *conf)
{
    lenet5_feature_t *features = fb_alloc0(sizeof(*features)); 
    load_input(features, src, roi);
    forward(lenet, features, relu);

    uint8_t result = 0;
    float *output = (float *)features->output; 
    float maxvalue = *output;

    for (uint8_t i=1; i < LENET_OUTPUT_SIZE; ++i) {
        if (output[i] > maxvalue) {
            maxvalue = output[i];
            result = i;
        }
    }

    *conf = output[result];
    fb_free();
    return result;
}

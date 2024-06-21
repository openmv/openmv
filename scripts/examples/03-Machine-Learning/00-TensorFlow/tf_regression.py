# This work is licensed under the MIT license.
# Copyright (c) 2013-2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# TensorFlow Lite Regression Example
#
# This example shows off running a regression model on the OpenMV Cam.
# A regression model takes an input list of numbers and produces an
# output list of numbers. You may pass 1D/2D/3D arrays to predict()
# and you will get a list of the results back.
#
# Note: The input list of numbers must be the same size as the input
# tensor size of the model. As mentioned before you can pass 1D/2D/3D arrays.

import tf

i = [-3, -1, -2, 5, -2, 10, -1, 9, 0, 2, 0, 9, 1, 10, 2, -1, 3,
     5, 3, 9, 3, 9, 6, 2, 6, 7, 5, 10, 6, -1, 7, 4, 7, 8,
     5, 7]

# Grab the model here:
# https://cdn.shopify.com/s/files/1/0803/9211/files/force_float32_quant.tflite?v=1718936445
m = tf.Model("force_float32_quant.tflite")
print(m)
print(m.predict(i))
# Should print 54.4129

# Grab the model here:
# https://cdn.shopify.com/s/files/1/0803/9211/files/force_int_quant.tflite?v=1718936445
m = tf.Model("force_int_quant.tflite")
print(m)
print(m.predict(i))
# Should print 53.7833

/**
  ******************************************************************************
  * @file    layers_list.h
  * @author  AST Embedded Analytics Research Platform
  * @brief   header file of AI platform layers datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018-2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* No sentry. This is deliberate!! */
/* Template: LAYER_ENTRY(type_, id_, struct_, forward_func_, init_func_, destroy_func_)
 * Where:
 *  - type_ is the (enum) type name of the layer. to have the complete enum
 *      value you should use the macro @ref AI_LAYER_TYPE_ENTRY(type_) that adds
 *      the specific prefix and postfix tokens to the type_
 *  - id_ is the numeric id of the layer
 *  - struct_ is the name of the datastruct of the layer without the ai_layer_
 *    prefix
 *  - forward_func_ is the forward function name of the routine implementing
 *        actual layer processing
 *  - init_func_ is the init function name of the routine implementing
 *        actual layer initialization
 *  - destroy_func_ is the destroy function name of the routine implementing
 *        actual layer de-initialization
 */

/* Layer IDs for stateless layers (bit 8 set) */
#define LAYER_ID(id_) \
  (0x100 + (id_))
/* Layer IDs for stateful layers (bits 7 and 8 set) */
#define LAYER_STATEFUL_ID(id_) \
  (0x180 + (id_))

/*!< Base layer */
LAYER_ENTRY(BASE, LAYER_ID(0), base, NULL, NULL, NULL)
/*!< Elementwise addition layer */
LAYER_ENTRY(ADD, LAYER_ID(1), add, forward_add, NULL, NULL)
 /*!< Batch normalization layer */
LAYER_ENTRY(BN, LAYER_ID(2), bn, forward_bn, NULL, NULL)
/*!< 2D Convolutional layer */
LAYER_ENTRY(CONV2D, LAYER_ID(3), conv2d, forward_conv2d, NULL, NULL)
/*!< Dense layer */
LAYER_ENTRY(DENSE, LAYER_ID(4), dense, forward_dense, NULL, NULL)
/*!< Local Response Normalization layer */
LAYER_ENTRY(LRN, LAYER_ID(6), lrn, forward_lrn, NULL, NULL)
/*!< Nonlinearity layer */
LAYER_ENTRY(NL, LAYER_ID(7), nl, NULL, NULL, NULL)
/*!< Normalization layer */
LAYER_ENTRY(NORM, LAYER_ID(8), norm, forward_norm, NULL, NULL)
/*!< Merged Conv2d / Pool layer */
LAYER_ENTRY(OPTIMIZED_CONV2D, LAYER_ID(9), conv2d_nl_pool, forward_conv2d_nl_pool, NULL, NULL)
/*!< Transpose Tensor layer */
LAYER_ENTRY(TRANSPOSE, LAYER_ID(10), transpose, forward_transpose, NULL, NULL)
/*!< Pooling layer */
LAYER_ENTRY(POOL, LAYER_ID(11), pool, forward_pool, NULL, NULL)
/*!< Softmax layer */
LAYER_ENTRY(SM, LAYER_ID(12), sm, forward_sm, NULL, NULL)
/*!< Split layer */
LAYER_ENTRY(SPLIT, LAYER_ID(13), split, forward_split, NULL, NULL)
/*!< TimeDelay layer */
LAYER_ENTRY(TIME_DELAY, LAYER_ID(14), time_delay, forward_time_delay, NULL, NULL)
/*!< TimeDistributed layer */
LAYER_ENTRY(TIME_DISTRIBUTED, LAYER_ID(15), time_distributed, forward_time_distributed, NULL, NULL)
/*!< Concat Tensor layer */
LAYER_ENTRY(CONCAT, LAYER_ID(16), concat, forward_concat, NULL, NULL)
/*!< GEMM layer */
LAYER_ENTRY(GEMM, LAYER_ID(17), gemm, forward_gemm, NULL, NULL)
/*!< Upsample layer */
LAYER_ENTRY(UPSAMPLE, LAYER_ID(18), upsample, forward_upsample, NULL, NULL)
/*!< Container layer for eltwise operations */
LAYER_ENTRY(ELTWISE, LAYER_ID(19), eltwise,  forward_eltwise, NULL, NULL)
/*!< Container layer for eltwise integer operations */
LAYER_ENTRY(ELTWISE_INTEGER, LAYER_ID(20), eltwise_integer,  NULL, NULL, NULL)
/*!< InstanceNormalization layer */
LAYER_ENTRY(INSTANCENORMALIZATION, LAYER_ID(21), instanceNormalization, forward_instanceNormalization, NULL, NULL)
/*!< Pad layer */
LAYER_ENTRY(PAD, LAYER_ID(22), pad,  forward_pad, NULL, NULL)
/*!< Slice layer */
LAYER_ENTRY(SLICE, LAYER_ID(23), slice, forward_slice, NULL, NULL)
/*!< Tile layer */
LAYER_ENTRY(TILE, LAYER_ID(24), tile, forward_tile, NULL, NULL)
/*!< Container layer for reduce operations */
LAYER_ENTRY(REDUCE, LAYER_ID(25), reduce,  forward_reduce, NULL, NULL)
/*!< Recurrent Neural Network layer */
LAYER_ENTRY(RNN, LAYER_ID(26), rnn, forward_rnn, NULL, NULL)
/*!< Resize layer */
LAYER_ENTRY(RESIZE, LAYER_ID(27), resize, forward_resize, NULL, NULL)
/*!< Gather layer */
LAYER_ENTRY(GATHER, LAYER_ID(28), gather, forward_gather, NULL, NULL)
/*!< Pack layer */
LAYER_ENTRY(PACK, LAYER_ID(29), pack, forward_pack, NULL, NULL)
/*!< Unpack layer */
LAYER_ENTRY(UNPACK, LAYER_ID(30), unpack, forward_unpack, NULL, NULL)
/*!< ArgMin & ArgMax layers */
LAYER_ENTRY(ARGMINMAX, LAYER_ID(31), argminmax, NULL, NULL, NULL)
/*!< Cast Neural Network Layer */
LAYER_ENTRY(CAST, LAYER_ID(33), cast, forward_cast, NULL, NULL)
/*!< iForest layer */
LAYER_ENTRY(IFOREST, LAYER_ID(34), iforest, forward_iforest, NULL, NULL)
/*!< SVM Regressor layer */
LAYER_ENTRY(SVMREG, LAYER_ID(35), svmreg, forward_svm_regressor, NULL, NULL)
/*!< ArrayFeatureExtractor layer */
LAYER_ENTRY(ARRAYFEATUREEXTRACTOR, LAYER_ID(36), arrayfeatureextractor, forward_arrayfeatureextractor, NULL, NULL)
/*!< SVM Classifier (SVC) layer */
LAYER_ENTRY(SVC, LAYER_ID(37), svc, forward_svc, NULL, NULL)
/*!< ZipMap layer */
LAYER_ENTRY(ZIPMAP, LAYER_ID(38), zipmap, forward_zipmap, NULL, NULL)
/*!< Where layer */
LAYER_ENTRY(WHERE, LAYER_ID(39), where, forward_where, NULL, NULL)
/*!< LinearClassifier layer */
LAYER_ENTRY(LINEARCLASSIFIER, LAYER_ID(40), linearclassifier, forward_linearclassifier, NULL, NULL)
/*!< TreeEnsembleClassifier layer */
LAYER_ENTRY(TREE_ENSEMBLE_CLASSIFIER, LAYER_ID(41), tree_ensemble_classifier, forward_tree_ensemble_classifier, NULL, NULL)
/*!< TopK layer */
LAYER_ENTRY(TOPK, LAYER_ID(42), topK, forward_topK, NULL, NULL)
/*!< ReduceLogSumExp layer */
LAYER_ENTRY(REDUCE_LOG_SUM_EXP, LAYER_ID(43), reduce_log_sum_exp, forward_reduce_log_sum_exp, NULL, NULL)
/*!< ReduceL1 layer */
LAYER_ENTRY(REDUCE_L1, LAYER_ID(44), reduce_l1, forward_reduce_l1, NULL, NULL)
/*!< Runtime Lite Graph Wrapper layer */
LAYER_ENTRY(LITE_GRAPH, LAYER_ID(45), lite_graph, NULL, NULL, NULL)
/*!< TreeEnsembleRegressor layer */
LAYER_ENTRY(TREE_ENSEMBLE_REGRESSOR, LAYER_ID(46), tree_ensemble_regressor, forward_tree_ensemble_regressor, NULL, NULL)
/*!< GatherND layer */
LAYER_ENTRY(GATHER_ND, LAYER_ID(67), gather_nd, forward_gather_nd, NULL, NULL)
/*!< MATMUL layer */
LAYER_ENTRY(MATMUL, LAYER_ID(68), matmul, forward_dmatmul, NULL, NULL)
/*!< Deeply Quantized Dense Layers */
LAYER_ENTRY(CONV2D_DQNN, LAYER_ID(48), conv2d_dqnn, forward_pw_is1os1ws1_bn, NULL, NULL)
LAYER_ENTRY(POOL_DQNN, LAYER_ID(49), pool_dqnn, forward_maxpool_is1os1, NULL, NULL)

LAYER_ENTRY(DENSE_DQNN, LAYER_ID(50), dense_dqnn, forward_dense_is1os1ws1, NULL, NULL)
/*!< Reverse layer */
LAYER_ENTRY(REVERSE, LAYER_ID(51), reverse, forward_reverse, NULL, NULL)
/*!< ScatterND layer */
LAYER_ENTRY(SCATTER_ND, LAYER_ID(69), scatter_nd, forward_scatter_nd, NULL, NULL)

/*!< TFLite wrapper */
LAYER_ENTRY(TFLITE_WRAPPER, LAYER_ID(52), tflite_wrapper, NULL, NULL, NULL)

/*****************************************************************************/
/*!< Base Stateful Layer type */
LAYER_ENTRY(STATEFUL, LAYER_STATEFUL_ID(0), stateful, NULL, NULL, NULL)
/*!< Long Short Time Memory layer */
LAYER_ENTRY(LSTM, LAYER_STATEFUL_ID(1), lstm, forward_lstm, init_lstm, destroy_lstm)
/*!< Custom layer */
LAYER_ENTRY(CUSTOM, LAYER_STATEFUL_ID(2), custom, NULL, NULL, NULL)
/*!< Gated Recurrent Unit layer */
LAYER_ENTRY(GRU, LAYER_STATEFUL_ID(3), gru, forward_gru, init_gru, destroy_gru)

/*!< Stateless Template layer declaration */
/* LAYER_ENTRY(TEMPLATE, LAYER_ID(XX), template, forward_template, NULL, NULL) */
/*!< Stateful Template layer declaration */
/* LAYER_ENTRY(TEMPLATE, LAYER_STATEFUL_ID(XX), template, forward_template, init_template, destroy_template) */

#undef LAYER_ENTRY
#undef LAYER_ID
#undef LAYER_STATEFUL_ID

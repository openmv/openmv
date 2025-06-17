/**
  ******************************************************************************
  * @file    layers_ml_treeensembleclassifier.h
  * @author  AIS
  * @brief   header file of AI platform TreeEnsembleClassifier datatypes
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021-2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LAYERS_TREE_ENSEMBLE_CLASSIFIER_H
#define LAYERS_TREE_ENSEMBLE_CLASSIFIER_H

#include "layers_common.h"
#include "layers_nl.h"



/*!
 * @defgroup layers_ml_treensembleclassifier Layers Definitions
 * @brief definition
 *
 */

AI_API_DECLARE_BEGIN

/* Error return codes */
#define AI_TREE_ENSEMBLE_CLASSIFIER_ERROR_NO                    0
#define AI_TREE_ENSEMBLE_CLASSIFIER_ERROR_WRONG_IDX_FMT         -1
#define AI_TREE_ENSEMBLE_CLASSIFIER_ERROR_UNFOUND_LEAF          -2
#define AI_TREE_ENSEMBLE_CLASSIFIER_ERROR_UNSUPPORTED_BRANCH    -3
#define AI_TREE_ENSEMBLE_CLASSIFIER_ERROR_UNSUPPORTED_FEATURE   -4

#define	AI_TREE_ENSEMBLE_CLASSIFIER_DEPTH_MAX                   10000


/* Type of condition in the TreeEnsembleClassifier*/
typedef enum
{
  AI_TREE_ENSEMBLE_CLASSIFIER_BRANCH_LT_IDX = 0,
  AI_TREE_ENSEMBLE_CLASSIFIER_BRANCH_LEQ_IDX,
  AI_TREE_ENSEMBLE_CLASSIFIER_BRANCH_EQ_IDX,
  AI_TREE_ENSEMBLE_CLASSIFIER_BRANCH_END,
} ai_tree_ensenble_classifier_branch_e;

typedef AI_ALIGNED_TYPE(struct, 4) ai_layer_tree_ensemble_classifier_ {
    AI_LAYER_COMMON_FIELDS_DECLARE
    func_nl nl_func;
    uint8_t all_weights_are_positive;
    ai_float nodes_values_scale;
    ai_float nodes_values_offset;
    ai_float class_weights_scale;
    ai_float class_weights_offset;
} ai_layer_tree_ensemble_classifier;



/******************************************************************************/
/*  Forward Functions Section                                                 */
/******************************************************************************/

/*!
 * @brief Decodes the TreeEnsembleClassifier ML operator.
 * @ingroup layers_svmreg
 * @param layer tree ensemble classifier layer
 */
AI_INTERNAL_API
void forward_tree_ensemble_classifier(ai_layer *pLayer);

AI_INTERNAL_API
ai_i32 decodeEstimator_LEQ_8Bits(const ai_float *pDataIn,
                                 ai_float *pOutDataScores,
                                 const ai_u8 *pFeatureIdxForEstimator,
                                 const ai_float *pValuesForEstimator,
                                 const ai_u8 *pTrueIdxForEstimator,
                                 const ai_u8 *pFalseIdxForEstimator,
                                 const ai_handle pClassWeightsForEstimator,
                                 const ai_array_format classWeightsFormat,
                                 const ai_u8 *pClassNodeIdsForEstimator,
                                 const ai_u16 nbClassWithCurrentEstimator,
                                 const ai_u8 *pClassIdsForEstimator);

AI_INTERNAL_API
ai_i32 decodeEstimator_LEQ_16Bits(const ai_float *pDataIn,
                                  ai_float *pOutDataScores,
                                  const ai_u8 *pFeatureIdxForEstimator,
                                  const ai_float *pValuesForEstimator,
                                  const ai_u16 *pTrueIdxForEstimator,
                                  const ai_u16 *pFalseIdxForEstimator,
                                  ai_handle pClassWeightsForEstimator,
                                  const ai_array_format classWeightsFormat,
                                  const ai_u16 *pClassNodeIdsForEstimator,
                                  const ai_u16 nbClassWithCurrentEstimator,
                                  const ai_u16 *pClassIdsForEstimator);



AI_API_DECLARE_END

#endif    /*LAYERS_TREE_ENSEMBLE_CLASSIFIER_H*/

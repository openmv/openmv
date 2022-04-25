# Tensorflow Support
The OpenMV firmware supports loading quantized Tensorflow Lite models. The firmware supports loading external models that reside on the filesystem to memory (on boards with SDRAM), and internal models (embedded into the firmware) in place. To load an external Tensorflow model from the filesystem from Python use [`tf`](https://docs.openmv.io/library/omv.tf.html) Python module. For information on embedding Tensorflow models into the firmware, and loading them, please see the following sections.
  - [Training Tensorflow models](#training-tensorflow-models)
  - [Embedding Tensorflow models in the firmware](#embedding-tensorflow-models-in-the-firmware)
  - [Loading embedded Tensorflow models](#loading-embedded-tensorflow-models)

## Training Tensorflow models
You can use Tensorflow Lite with any high-level API, such as Keras or pytorch, to train and quantize models (for example, see [Training Mobilenet V2 for OpenMV with Keras](https://github.com/SingTown/openmv_tensorflow_training_scripts/blob/main/openmv_mobilenet_v2.ipynb)). However, the officially supported way to train models for OpenMV cameras is using EdgeImpulse. For more information on using EdgeImpulse with OpenMV cameras, please see the [EdgeImpulse tutorial for OpenMV](https://docs.edgeimpulse.com/docs/openmv-cam-h7-plus).

## Embedding Tensorflow models in the firmware
Tensorflow models can be embedded into the firmware for boards that don't have an SDRAM. The firmware ships with a default embedded model, that can be replaced by any other Tensorflow model. Additionally, more models can be embedded in the firmware, as many as the free flash size allows. Once you have a tflite model to replace the default model, or to add to the firmware, it can be embedded in the firmware by placing the model `.tflite` file and its `.txt` labels file in `src/libtf/models`. The model file and labels file must have the same basename, for example:

```bash
models/mask_detection.txt
models/mask_detection.tflite
```

The labels `.txt` file contains model output labels arranged 1 label per line, for example:
```
>>cat mask_detection.txt 
mask
no_mask
```

After replacing/adding a Tensorflow model, rebuild the firmware to use the new model. The build system will automatically detect the models and convert them to C structs that get embedded into the firmware image. For more information on how to build the OpenMV firmware from source, see [Building the Firmware From Source](https://github.com/openmv/openmv/blob/master/src/README.md)

## Loading embedded Tensorflow models

Built-in models can be loaded with `tf.load_builtin_model()` function, for example the following loads the built-in person detection model:
```Python
labels, net = tf.load_builtin_model('person_detection')
```

For more information on how to OpenMV Tensorflow Python module, see the [documentation](https://docs.openmv.io/library/omv.tf.html)

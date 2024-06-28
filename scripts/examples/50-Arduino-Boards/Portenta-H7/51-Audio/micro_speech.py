# This work is licensed under the MIT license.
# Copyright (c) 2024 OpenMV LLC. All rights reserved.
# https://github.com/openmv/openmv/blob/master/LICENSE
#
# The MicroSpeech module is designed for real-time audio processing and speech recognition
# on microcontroller platforms. It leverages pre-trained models for audio preprocessing and
# speech recognition, specifically optimized for detecting keywords such as "Yes" and "No".
import ml
import time


def callback(label, scores):
    print(f'\nHeard: "{label}" @{time.ticks_ms()}ms Scores: {scores}')


# By default, the MicroSpeech object uses the built-in audio preprocessor (float) and the
# micro speech module for audio preprocessing and speech recognition, respectively. The
# user can override both by passing two models:
# MicroSpeech(preprocessor=ml.Model(...), micro_speech=ml.Model(...), labels=["label",...])
speech = ml.MicroSpeech()

# Starts the audio streaming and processes incoming audio to recognize speech commands.
# If a callback is passed, listen() will loop forever and call the callback when a keyword
# is detected. Alternatively, `listen()` can be called with a timeout (in ms), and it
# returns if the timeout expires before detecting a keyword.
speech.listen(callback=callback, threshold=0.70)

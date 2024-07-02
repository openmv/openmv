# This file is part of the OpenMV project.
#
# Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# This is an extension to the display C user-module. Add or import any display-related
# drivers here, and freeze this module in the board's manifest, and those drivers will
# be importable from display.

import time
from uml import *  # noqa
from micropython import const
from ulab import numpy as np
try:
    import audio
except (ImportError, AttributeError):
    pass


def draw_predictions(img, boxes, labels, colors, format="pascal_voc", text_color=(255, 255, 255)):
    CHAR_W = 8
    CHAR_H = 10
    img_w = img.width()
    img_h = img.height()
    for i, (x, y, w, h) in enumerate(boxes):
        label = labels[i]
        box_color = colors[i]

        if format == "pascal_voc":
            x = int(x * img_w)
            y = int(y * img_h)
            w = int(w * img_w) - x
            h = int(h * img_h) - y

        img.draw_rectangle(x, y, w, h, color=box_color)
        img.draw_rectangle(
            x, y - CHAR_H, len(label) * CHAR_W, CHAR_H, fill=True, color=box_color
        )
        img.draw_string(x, y - CHAR_H, label.upper(), text_color)


class MicroSpeech:
    _SLICE_SIZE = const(40)
    _SLICE_COUNT = const(49)
    _SLICE_TIME_MS = const(30)
    _AUDIO_FREQUENCY = const(16000)
    _SAMPLES_PER_STEP = const(10 * (_AUDIO_FREQUENCY // 1000))  # 10ms * 16 Samples/ms
    _CATEGORY_COUNT = const(4)
    _AVERAGE_WINDOW_SAMPLES = const(1020 // _SLICE_TIME_MS)

    def __init__(self, preprocessor=None, micro_speech=None, labels=None):
        self.preprocessor = preprocessor
        if preprocessor is None:
            self.preprocessor = Model("audio_preprocessor")[1]
        self.labels, self.micro_speech = (labels, micro_speech)
        if micro_speech is None:
            self.labels, self.micro_speech = Model("micro_speech")
        # 16 samples/1ms
        self.audio_buffer = np.zeros((1, _SAMPLES_PER_STEP * 3), dtype=np.int16)
        self.spectrogram = np.zeros((1, _SLICE_COUNT * _SLICE_SIZE), dtype=np.int8)
        self.pred_history = np.zeros((_AVERAGE_WINDOW_SAMPLES, _CATEGORY_COUNT), dtype=np.float)
        self.audio_started = False
        audio.init(channels=1, frequency=_AUDIO_FREQUENCY, gain_db=24, samples=_SAMPLES_PER_STEP * 2)

    def audio_callback(self, buf):
        # Roll the audio buffer to the left, and add the new samples.
        self.audio_buffer = np.roll(self.audio_buffer, -(_SAMPLES_PER_STEP * 2), axis=1)
        self.audio_buffer[0, _SAMPLES_PER_STEP:] = np.frombuffer(buf, dtype=np.int16)

        # Roll the spectrogram to the left and add the new slice.
        self.spectrogram = np.roll(self.spectrogram, -_SLICE_SIZE, axis=1)
        self.spectrogram[0, -_SLICE_SIZE:] = self.preprocessor.predict(
            self.audio_buffer
        )

        # Roll the prediction history and add the new prediction.
        self.pred_history = np.roll(self.pred_history, -1, axis=0)
        self.pred_history[-1] = self.micro_speech.predict(self.spectrogram)[0]

    def start_audio_streaming(self):
        if self.audio_started is False:
            self.spectrogram[:] = 0
            self.pred_history[:] = 0
            audio.start_streaming(self.audio_callback)
            self.audio_started = True

    def stop_audio_streaming(self):
        audio.stop_streaming()
        self.audio_started = False

    def listen(self, timeout=0, callback=None, threshold=0.65, filter=["Yes", "No"]):
        self.start_audio_streaming()
        stat_ms = time.ticks_ms()
        while True:
            average_scores = np.mean(self.pred_history, axis=0)
            max_score_index = np.argmax(average_scores)
            max_score = average_scores[max_score_index]
            label = self.labels[max_score_index]
            if max_score > threshold and label in filter:
                self.pred_history[:] = 0
                self.spectrogram[:] = 0
                if callback is None:
                    if timeout != -1:  # non-blocking mode
                        self.stop_audio_streaming()
                    return (label, average_scores)
                callback(label, average_scores)
            if timeout == -1:  # non-blocking mode
                return (None, average_scores)
            if timeout != 0 and (time.ticks_ms() - stat_ms) > timeout:
                self.stop_audio_streaming()
            time.sleep_ms(1)

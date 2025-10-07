# Copyright (C) 2025 OpenMV, LLC.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Any redistribution, use, or modification in source or binary form
#    is done solely for personal benefit and not for any commercial
#    purpose or for monetary gain. For commercial licensing options,
#    please contact openmv@openmv.io
#
# THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
# OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Import EdgeImpulse FOMO postprocessing from subpackage for backwards compatibility
from ml.postprocessing.edgeimpulse import Fomo as fomo_postprocess  # noqa

# Import Darknet YOLO postprocessing from subpackage for backwards compatibility
from ml.postprocessing.darknet import YoloV2 as yolo_v2_postprocess  # noqa
from ml.postprocessing.darknet import YoloLC as yolo_lc_postprocess  # noqa

# Import Ultralytics YOLO postprocessing from subpackage for backwards compatibility
from ml.postprocessing.ultralytics import YoloV5 as yolo_v5_postprocess  # noqa
from ml.postprocessing.ultralytics import YoloV8 as yolo_v8_postprocess  # noqa

# Import mediapipe postprocessing from subpackage for backwards compatibility
from ml.postprocessing.mediapipe import BlazeFace as mediapipe_face_detection_postprocess  # noqa

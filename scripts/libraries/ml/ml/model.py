# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
import uml
import image
from ml.preprocessing import Normalization


class Model(uml.Model):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def predict(self, args, **kwargs):
        args = [Normalization()(x) if isinstance(x, image.Image) else x for x in args]
        return super().predict(args, **kwargs)

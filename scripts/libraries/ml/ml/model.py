# This file is part of the OpenMV project.
#
# Copyright (c) 2024 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2024 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
import uml
import image
from ml.preprocessing import Normalization


class Model:
    def __new__(cls, *args, **kwargs):
        self = super().__new__(cls)
        retobj = uml.Model(*args, **kwargs)
        if isinstance(retobj, tuple):
            labels, self.model = retobj
            return labels, self
        return self

    def __str__(self):
        return str(self.model)

    def predict(self, args, **kwargs):
        args = [Normalization()(x) if isinstance(x, image.Image) else x for x in args]
        return self.model.predict(args, **kwargs)

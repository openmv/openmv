class ADPS9960InvalidDevId(ValueError):
    def __init__(self, id, valid_ids):
        Exception.__init__(
            self,
            "Device id 0x{} is not a valied one (valid: {})!".format(
                format(id, "02x"), ", ".join(["0x{}".format(format(i, "02x")) for i in valid_ids])
            ),
        )


class ADPS9960InvalidMode(ValueError):
    def __init__(self, mode):
        Exception.__init__(self, "Feature mode {} is invalid!".format(mode))

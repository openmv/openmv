def clamp_to_bytes(value, nbytes):
    """Clamp integer to the max storable value for nbytes."""
    if value is None:
        value = 0
    if value < 0:
        value = 0
    max_val = (1 << (8 * nbytes)) - 1
    if value > max_val:
        return max_val
    return value


def int_to_nbytes(value, nbytes):
    v = clamp_to_bytes(int(value), nbytes)
    return v.to_bytes(nbytes, "big")

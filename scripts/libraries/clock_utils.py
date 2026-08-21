import utime


def get_epoch_ms():  # unix epoch milliseconds, eg. 1381791310000
    return utime.time_ns() // 1_000_000


def get_epoch_sec():  # unix epoch seconds, eg. 1736931600
    return int(utime.ticks_ms() / 1000)

def format_epochms_str(epoch_ms):
    if epoch_ms is None:
        return None
    y, mo, d, h, mi, s, _, _ = utime.gmtime(int(epoch_ms) // 1000)
    return "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(y, mo, d, h, mi, s)

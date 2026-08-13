import utime


def get_epoch_ms():  # unix epoch milliseconds, eg. 1381791310000
    return utime.time_ns() // 1_000_000


def get_epoch_sec():  # unix epoch seconds, eg. 1736931600
    return int(utime.ticks_ms() / 1000)

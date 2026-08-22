from collections import deque
from config import save_log_entry
from clock_utils import get_epoch_ms

log_q_len = 100
saved_logs = deque([], log_q_len)

def get_saved_logs():
    return saved_logs
def return_saved_logs_and_clear():
    global saved_logs
    to_ret = saved_logs
    saved_logs = deque([], log_q_len)
    return to_ret

def log_internal(m):
    print(m)
    saved_logs.append(m)

def info(m):
    log_internal(f"[info] : {m}")
def debug(m):
    pass
    # log_internal(f"[debug] : {m}")
def warning(m):
    log_internal(f"[WARNING] : {m}")
def error(m):
    log_internal(f"[ERROR] : {m}")
def fatal(m):
    log_internal(f"[FATAL] : {m}")
    try:
        epoch_ms = get_epoch_ms()
        save_log_entry(f'{epoch_ms} [FATAL]: {m}')
    except Exception as e:
        pass

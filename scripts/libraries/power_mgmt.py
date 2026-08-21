"""
Camera power save via OpenMV sensor.sleep + skip_frames (docs v4.5.6):

  sensor.sleep(True)   — sleep camera when not capturing
  sensor.sleep(False)  — wake camera
  sensor.skip_frames() — stabilize pipeline before snapshot (default 300 ms)

Keep camera awake for a whole burst; sleep only between capture events.
"""
import sensor
import logger

_asleep = False


def is_asleep():
    return _asleep


def camera_sleep():
    """Put camera to sleep when idle."""
    global _asleep
    if _asleep:
        return
    try:
        sensor.sleep(True)
        _asleep = True
    except Exception as e:
        logger.warning(f"[PWR] camera_sleep failed: {e}")


def camera_wake():
    """
    Wake camera and skip_frames so snapshot() can succeed.
    No-op if already awake (safe across multi-frame burst).
    """
    global _asleep
    if not _asleep:
        return
    try:
        sensor.sleep(False)
        # Docs: call after settings/state change; default 300 ms.
        # HD needs a bit more settle time than QVGA examples.
        sensor.skip_frames(time=500)
        _asleep = False
    except Exception as e:
        logger.warning(f"[PWR] camera_wake/skip_frames failed ({e}); re-init")
        try:
            sensor.reset()
            sensor.set_pixformat(sensor.RGB565)
            sensor.set_framesize(sensor.HD)
            sensor.skip_frames(time=1000)
            _asleep = False
        except Exception as e2:
            logger.error(f"[PWR] camera re-init failed: {e2}")


def system_can_power_save(
    trans_in_progress=False,
    pir_burst_in_progress=False,
    is_install_mode=False,
    packet_queue_len=0,
):
    """True when safe to idle CPU without disrupting LoRa/PIR/transfer."""
    return not (
        trans_in_progress
        or pir_burst_in_progress
        or is_install_mode
        or packet_queue_len > 0
    )

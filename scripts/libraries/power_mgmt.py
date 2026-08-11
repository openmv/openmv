"""RT1062 power helpers: machine.idle() only (sensor.sleep breaks snapshot after wake)."""
import machine


def system_can_power_save(
    trans_in_progress=False,
    pir_burst_in_progress=False,
    is_install_mode=False,
    packet_queue_len=0,
):
    """True when safe to idle without disrupting LoRa/PIR/transfer."""
    return not (
        trans_in_progress
        or pir_burst_in_progress
        or is_install_mode
        or packet_queue_len > 0
    )

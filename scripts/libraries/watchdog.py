"""
Hardware watchdog timer (WDT) for OpenMV Cam RT1062.

Once started, the MCU countdown must be fed before timeout or the board reboots.
On RT1062 the watchdog cannot be stopped after start; timeout is set at create time.

Run this file alone on the board to test:
    import watchdog
    # or open watchdog.py in OpenMV IDE and run
"""
from machine import WDT
import machine
import time
import uasyncio as asyncio
import logger

WDT_TIMEOUT_MS = 90_000
wdt = None
_active_timeout_ms = WDT_TIMEOUT_MS


def feed_watchdog():
    """Reset the WDT countdown if the watchdog has been started."""
    global wdt
    if wdt is not None:
        wdt.feed()


async def watchdog_feed_loop():
    """Background task: feed WDT every one-third of the active timeout."""
    feed_interval_ms = _active_timeout_ms // 3
    while True:
        feed_watchdog()
        await asyncio.sleep_ms(feed_interval_ms)


def start_watchdog(timeout_ms=None):
    """
    Create the hardware WDT and spawn the background feed task.
    Call after init_device() so boot-time work does not trip a false reset.

    timeout_ms: optional override (default WDT_TIMEOUT_MS). Useful for short tests.
    """
    global wdt, _active_timeout_ms
    _active_timeout_ms = timeout_ms if timeout_ms is not None else WDT_TIMEOUT_MS
    wdt = WDT(timeout=_active_timeout_ms)
    asyncio.create_task(watchdog_feed_loop())
    logger.info(f"Watchdog started (timeout={_active_timeout_ms} ms)")


def check_watchdog_reset():
    """Log if the previous boot ended because the WDT expired."""
    if machine.reset_cause() == machine.WDT_RESET:
        logger.fatal("✘✘✘ Previous boot ended in watchdog reset ✘✘✘")
        return True
    return False


# ---------------------------------------------------------------------------
# Standalone test — run this file on the OpenMV board
# ---------------------------------------------------------------------------
# TEST_REBOOT = False  → only check that feeding keeps the board alive
# TEST_REBOOT = True   → then stop feeding; board should auto-reboot
TEST_REBOOT = True
TEST_TIMEOUT_MS = 5_000  # short timeout so the test finishes quickly


async def main():
    print("")
    print("========== WATCHDOG TEST ==========")
    print(f"timeout = {TEST_TIMEOUT_MS} ms")
    print(f"feed every = {TEST_TIMEOUT_MS // 3} ms")
    print(f"test reboot = {TEST_REBOOT}")
    print("===================================")

    # --- Check why we booted ---
    if check_watchdog_reset():
        print("BOOT: last reset was WDT (reboot test passed last run)")
    else:
        print("BOOT: normal power-on / soft reset")

    # --- Test 1: feed keeps device alive ---
    print("")
    print("TEST 1: start WDT and keep feeding")
    start_watchdog(timeout_ms=TEST_TIMEOUT_MS)

    # Stay alive for 3x timeout while the background feed loop runs
    for sec in range(1, 4):
        await asyncio.sleep_ms(TEST_TIMEOUT_MS)
        print(f"  alive after {sec * TEST_TIMEOUT_MS // 1000}s")
    print("TEST 1 PASS: feeding works")

    if not TEST_REBOOT:
        print("")
        print("Done. Set TEST_REBOOT = True to test auto-reboot.")
        return

    # --- Test 2: no feed → hardware reboot ---
    print("")
    print(f"TEST 2: stop feeding — expect reboot in ~{TEST_TIMEOUT_MS // 1000}s")
    print("  (after reboot, run this script again; BOOT should say WDT)")
    # Blocking sleep freezes asyncio, so feed loop cannot run
    time.sleep_ms(TEST_TIMEOUT_MS + 2_000)

    print("TEST 2 FAIL: board did not reboot")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Watchdog test interrupted")

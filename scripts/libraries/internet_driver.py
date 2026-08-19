import time
import utime
from config import (
    get_my_addr,
    led_restart_blinker,
    ENCRYPTION_ENABLED,
    uses_hybrid_encryption,
)
import os
import machine
import sensor
import ubinascii
import enc
import uasyncio as asyncio
from message_codec import build_heartbeat_payload
from detect import turn_ON_IR_emitter, turn_OFF_IR_emitter

try:
    import logger
    HAS_LOGGER = True
except ImportError:
    HAS_LOGGER = False

    class SimpleLogger:
        def debug(self, msg):
            print(f"[DEBUG] {msg}")

        def info(self, msg):
            print(f"[INFO] {msg}")

        def error(self, msg):
            print(f"[ERROR] {msg}")

        def warning(self, msg):
            print(f"[WARN] {msg}")

    logger = SimpleLogger()

# Try to import MicroPython UART (like internet_driver.py); fall back to desktop serial-only
try:
    from machine import UART  # type: ignore

    HAS_MACHINE_UART = True
except ImportError:
    HAS_MACHINE_UART = False

# Connection configuration (aligned with internet_driver.py style)
# On the MicroPython side we use UART_ID/BAUDRATE; here we map to the host serial port.

# Configuration
UART_ID = 1
BAUDRATE = 115200
INTERNET_RECHECK_INTERVAL_SEC = 60 * 60  # 60 minutes

# AT response substrings (CEREG=LTE, CGREG=3G/2G PS) and saved nw_type (0–4)
NW_LABEL_LTE_HOME = "+CEREG: 0,1"
NW_LABEL_LTE_ROAM = "+CEREG: 0,5"
NW_LABEL_3G_PS_HOME = "+CGREG: 0,1"
NW_LABEL_3G_PS_ROAM = "+CGREG: 0,5"

NW_TYPE_UNKNOWN = 0
NW_TYPE_LTE_HOME = 1
NW_TYPE_LTE_ROAM = 2
NW_TYPE_3G_PS_HOME = 3
NW_TYPE_3G_PS_ROAM = 4

NW_TYPE_NAMES = {
    NW_TYPE_UNKNOWN: "UNKNOWN",
    NW_TYPE_LTE_HOME: "LTE_HOME",
    NW_TYPE_LTE_ROAM: "LTE_ROAM",
    NW_TYPE_3G_PS_HOME: "3G_PS_HOME",
    NW_TYPE_3G_PS_ROAM: "3G_PS_ROAM",
}

_NW_LABEL_TO_TYPE = (
    (NW_LABEL_LTE_HOME, NW_TYPE_LTE_HOME),
    (NW_LABEL_LTE_ROAM, NW_TYPE_LTE_ROAM),
    (NW_LABEL_3G_PS_HOME, NW_TYPE_3G_PS_HOME),
    (NW_LABEL_3G_PS_ROAM, NW_TYPE_3G_PS_ROAM),
)


class _UARTSerialAdapter:
    def __init__(self, uart):
        self._uart = uart

    @property
    def in_waiting(self):
        # MicroPython UART.any() returns number of bytes available
        return self._uart.any()

    def read(self, n):
        return self._uart.read(n)

    def write(self, data):
        return self._uart.write(data)


class InternetDriver:
    def __init__(
        self,
        uart=None,
        context_id=1,
        uart_id=1,
        baudrate=BAUDRATE,
        configure_sensor=False,
        process_id="XYZ",
    ):
        """
        Initialize EC200 HTTP client

        Args:
            uart: Serial-like object or MicroPython UART. If None, a UART
                         will be created on MicroPython using uart_id/baudrate.
            context_id: PDP context ID (default: 1)
            timeout: Default timeout for commands in seconds
            uart_id: UART ID to use when creating a MicroPython UART (default: 1)
            baudrate: Baudrate when creating a MicroPython UART (default: 115200)
            machine_id: Device address for payloads

        Attributes:
            has_internet: True only if PDP/HTTP init succeeded within 3 tries and
                make_upload_test() passed (≥2/3 uploads); else False.
            configure_sensor: True if the sensor should be configured, False otherwise. (as done main)
        """
        try:
            self.machine_id = get_my_addr()
            self.configure_sensor = configure_sensor
            self.process_id = process_id
            if self.configure_sensor:
                sensor.reset()
                sensor.set_pixformat(sensor.RGB565)
                sensor.set_framesize(sensor.SVGA)
                sensor.skip_frames(time=2000)
            # If caller passed a MicroPython UART (like main.py does), adapt it.
            self.initialized = False
            if uart is not None:
                if hasattr(uart, "any") and not hasattr(uart, "in_waiting"):
                    # Looks like a raw MicroPython UART instance
                    self.uart = _UARTSerialAdapter(uart)
                else:
                    # Already a desktop-style serial object (has in_waiting)
                    self.uart = uart
            else:
                # No serial passed in – try to create one like internet_driver.py does.
                if HAS_MACHINE_UART:
                    uart = UART(uart_id, baudrate, timeout=2000)
                    # Give the module a moment after UART comes up (like internet_driver.py)
                    try:
                        time.sleep_ms(2000)
                    except Exception:
                        pass
                    self.uart = _UARTSerialAdapter(uart)
                    print("using UART port")
                else:
                    # Desktop/host environment: expect a serial-like object to be passed in
                    print("ERROR - UART not available and no uart provided")
                    raise Exception("ERROR - UART not available and no uart provided")

            self.context_id = context_id
            self.default_timeout = 10  # 10 seconds
            self._last_recovery_fail_ticks = 0  # time.ticks_ms() when recovery last failed
            self._uploads_since_health_check = 10  # trigger health+CSQ on first upload
            self._health_check_interval = 10
            self._upload_success_count = 0
            self._upload_fail_count = 0
            self._last_fail_count = 0
            self.is_busy = False

            self.has_internet = False
            self.signal_strength = 0
            self.network_type = NW_TYPE_UNKNOWN
            logger.info("InternetDriver init finished")
        except Exception as e:
            print(f"Error in InternetDriver init: {e}")
            self.has_internet = False

    # ------------------------------------------------------------------
    # Core UART helpers
    # ------------------------------------------------------------------

    async def _send_command(self, command, wait_for="OK", timeout=None):
        """
        Send AT command and wait for response.
        Drain stale bytes with a single fast pass, and send AT command
        """
        if timeout is None:
            timeout = self.default_timeout

        # Clears leftover UART input so the next command’s response is not mixed with stale output from earlier traffic.
        try:
            drain_deadline = time.ticks_ms() + 200
            while time.ticks_diff(drain_deadline, time.ticks_ms()) > 0:
                if getattr(self.uart, "in_waiting", 0):
                    self.uart.read(self.uart.in_waiting)
                    await asyncio.sleep(0.02)  # give module 20 ms to push any remaining bytes
                else:
                    break  # buffer empty - no need to wait the full 200 ms
        except Exception:
            pass

        # Send command
        self.uart.write((command + "\r\n").encode())
        return await self._read_response(wait_for, int(timeout * 1000))

    async def _read_response(self, wait_for, timeout_ms, error_str="ERROR"):
        """
        Low-level polling reader. Returns (found: bool, response: str).
        Uses tight 20 ms sleep instead of 100 ms to reduce latency.
        """
        deadline = time.ticks_ms() + timeout_ms
        response = ""
        while time.ticks_diff(deadline, time.ticks_ms()) > 0:
            available = getattr(self.uart, "in_waiting", 0)
            if available:
                chunk = self.uart.read(available)
                if chunk:
                    try:
                        # MicroPython's bytes.decode may not support keyword arguments like 'errors'
                        response += chunk.decode("utf-8")
                    except Exception:
                        # Fallback: treat as latin-1 without keyword args for broad compatibility
                        try:
                            response += chunk.decode("latin-1")
                        except Exception:
                            # As a last resort, ignore undecodable bytes
                            pass
                if wait_for and wait_for in response:
                    return True, response
                if error_str and error_str in response:
                    return False, response
            await asyncio.sleep(0.02)
        return False, response

    async def _write_and_drain(self, data, drain_sleep=0.15):
        """
        Write raw bytes to UART and wait for TX to clock out.
        The EC200U-CN at 115200 baud
        clocks 512 bytes in ~45 ms; 150 ms gives 3× headroom without wasting time.
        If the caller knows its payload size it can pass a tighter value.
        """
        self.uart.write(data)
        if hasattr(self.uart, "flush"):
            try:
                self.uart.flush()
            except Exception:
                pass
        await asyncio.sleep(drain_sleep)

    # ------------------------------------------------------------------
    # Initialization & configuration
    # ------------------------------------------------------------------

    async def initialize_internet(self):
        """One-time initialization and configuration.

        Returns:
            bool: True on successful initialization, False on failure.
            error: Message or Error message in case of success and failure
        """
        print("[CELL] Initializing EC200 HTTP client...")

        # Let module settle (e.g. after GPS/cellular handover, pending output)
        await asyncio.sleep(0.5)

        # Give module multiple chances to respond to basic AT (EC200 can be slow after heavy use)
        for attempt in range(5):
            success, _ = await self._send_command("AT", timeout=5)
            if success:
                break
            await asyncio.sleep(2)  # Longer delay between retries - module may need time
        else:
            self.initialized = False
            return False, "Failed to send AT command!"

        # Ensure PDP context is active. Deactivate first to clear any stale/partial
        # state, then re-activate cleanly. Deactivation errors are ignored - the
        # context may already be inactive. This must live here (not only in
        # is_healthy) so that all recovery paths are self-contained.

        # Give the module time to fully reset or QIACT may appear OK but drop immediately and cause AT+QHTTPURL 711 errors.
        print("[CELL] Activating PDP context...")
        await asyncio.sleep(3)
        await self._send_command(f"AT+QIDEACT={self.context_id}", timeout=10)
        await asyncio.sleep(1)
        success, resp = await self._send_command(f"AT+QIACT={self.context_id}", timeout=30)
        if not success:
            self.initialized = False
            return False, f"Failed to activate PDP context: {resp}"

        # Verify context is actually up before continuing
        success, resp = await self._send_command("AT+QIACT?", timeout=5)
        if not success or f"+QIACT: {self.context_id},1" not in resp:
            self.initialized = False
            return False, f"PDP context did not come up cleanly: {resp}"
        print("[CELL] PDP context active")

        # Reset HTTP context flag so it gets configured fresh after (re-)init
        if not await self._configure_http_context():
            self.initialized = False
            return False, "Failed to configure HTTP context"

        self.initialized = True
        print("EC200 HTTP client initialized successfully")
        return True, None

    async def establish_internet(self, retry_count=3):
        init_ok = False
        init_error = None
        for attempt in range(1, retry_count+1):
            init_success, init_error = await self.initialize_internet()
            if init_success:
                init_ok = True
                break
            print(f"[CELL] Internet init failed (attempt {attempt}/{retry_count}): {init_error}")
            if attempt < retry_count:
                await asyncio.sleep(2)
        if not init_ok:
            self.has_internet = False
            print(f"[ERROR] : [CELL] Internet init failed after {retry_count} attempts: {init_error}")
        else:
            upload_ok = await self.make_upload_test()
            self.has_internet = upload_ok
            if not upload_ok:
                print("[CELL] has_internet=False: upload validation failed (<2/3 OK)")

    async def _configure_http_context(self):
        """
        Configure HTTP context settings.
        upload_data() calls can skip redundant reconfiguration.
        """
        # Configure HTTP context (bind to PDP context)
        success, resp = await self._send_command(f'AT+QHTTPCFG="contextid",{self.context_id}')
        if not success:
            print(f"Failed to set context ID: {resp}")
            return False

        # Configure simple JSON header behavior
        # Disable per-request header mode and set a fixed Content-Type header.
        success, resp = await self._send_command('AT+QHTTPCFG="requestheader",0')
        if not success:
            print(f"Failed to configure requestheader: {resp}")
            return False

        success, resp = await self._send_command(
            'AT+QHTTPCFG="header","Content-Type: application/json"'
        )
        if not success:
            print(f"Failed to set default header: {resp}")
            return False

        return True

    async def _http_stop(self):
        """Best-effort: clear stuck Quectel HTTP state before a new QHTTPURL/QHTTPPOST."""
        try:
            await self._send_command("AT+QHTTPSTOP", timeout=5)
        except Exception:
            pass
        await asyncio.sleep(0.2)
    
    # ------------------------------------------------------------------
    # Health & diagnostics
    # ------------------------------------------------------------------

    def _network_type_from_responses(self, resp):
        """Map a single CEREG/CGREG UART response text to NW_TYPE_*."""
        if not resp:
            return NW_TYPE_UNKNOWN
        for label, nw_type in _NW_LABEL_TO_TYPE:
            if label in resp:
                return nw_type
        return NW_TYPE_UNKNOWN

    async def is_healthy(self):
        """
        Lightweight health check.
        Combines CEREG(LTE) or CGREG(3G/2G PS) + QIACT into a single pass; skips the bare AT ping
        (CEREG already proves the module is alive and responding).
        """
        # AT+CEREG proves the module is alive AND confirms LTE data registration
        success, resp = await self._send_command("AT+CEREG?", timeout=4)
        if not success:
            return False

        nw_type = self._network_type_from_responses(resp)
        if nw_type == NW_TYPE_UNKNOWN:
            success2, resp2 = await self._send_command("AT+CGREG?", timeout=4)
            if not success2:
                self.save_network_type(NW_TYPE_UNKNOWN)
                return False
            nw_type = self._network_type_from_responses(resp2)
            if nw_type == NW_TYPE_UNKNOWN:
                print("Not registered on LTE or GPRS (3G/2G PS)")
                self.save_network_type(NW_TYPE_UNKNOWN)
                return False
        self.save_network_type(nw_type)

        # PDP context check
        success, resp = await self._send_command("AT+QIACT?", timeout=4)
        if not success or f"+QIACT: {self.context_id},1" not in resp:
            print("PDP context not active, triggering full recovery...")
            return False

        print("Connection is healthy")
        return True

    async def get_signal_strength(self):
        """
        Get cellular signal strength (0-100 %) via AT+CSQ.
        Called only periodically now — not before every upload.
        """
        success, resp = await self._send_command("AT+CSQ", timeout=3)
        if not success or "+CSQ:" not in resp:
            return None
        try:
            for line in resp.split("\n"):
                line = line.strip()
                if line.startswith("+CSQ:"):
                    parts = line.split(":")[1].strip().split(",")
                    rssi = int(parts[0].strip())
                    if rssi == 99:
                        return None
                    if 0 <= rssi <= 31:
                        return min(100, int(round((rssi / 31) * 100)))
        except (ValueError, IndexError):
            pass
        return None
    
    def save_signal_strength(self, signal_strength):
        try:
            if signal_strength is None:
                self.signal_strength = 0
            else:
                self.signal_strength = signal_strength
        except Exception as e:
            logger.error(f"[CELL] Error saving signal strength: {e}")
            self.signal_strength = 0
    
    def get_last_signal_strength(self):
        return self.signal_strength

    def save_network_type(self, network_type):
        if network_type not in NW_TYPE_NAMES:
            network_type = NW_TYPE_UNKNOWN
        self.network_type = network_type
        logger.info(
            f"[CELL] Found network type: {network_type} ({NW_TYPE_NAMES.get(network_type, 'UNKNOWN')})"
        )

    def get_last_network_type(self):
        return self.network_type

    # ------------------------------------------------------------------
    # Upload
    # ------------------------------------------------------------------

    async def upload_data(
        self, data, url, headers=None, input_timeout=20, response_timeout=20
    ):
        """
        Make a POST request with minimised per-request overhead.

        Key optimisations vs. original:
        - Health check runs every _health_check_interval uploads (not every time).
        - Signal strength is logged every _health_check_interval uploads (not every time).
        - HTTP context is configured once and cached (_http_context_configured flag).
        - drain_sleep reduced (150 ms vs 500 ms).
        - Poll sleep reduced (20 ms vs 100 ms).
        - Settling gap after URL removed (was 300 ms dead sleep).
        - URL OK-wait loop uses the same fast _read_response helper.
        """
        # --- Periodic health check (not every single upload) ---
        self.is_busy = True
        self._uploads_since_health_check += 1
        do_health_check = (
            self._uploads_since_health_check > self._health_check_interval
        )
        if do_health_check:
            self._uploads_since_health_check = 0

        if do_health_check:
            for health_attempt in range(2):
                if await self.is_healthy():
                    break
                if health_attempt == 0:
                    await asyncio.sleep(1)
            else:
                recovery_cooldown_sec = 20
                now_ms = time.ticks_ms()
                if self._last_recovery_fail_ticks:
                    elapsed_ms = time.ticks_diff(now_ms, self._last_recovery_fail_ticks)
                    if elapsed_ms < recovery_cooldown_sec * 1000:
                        self.on_upload_fail()
                        self.is_busy = False
                        return False, 0, "Connection failed (recovery cooldown)"
                print("[CELL] Connection not healthy, attempting to recover...")
                await asyncio.sleep(2)
                init_success, init_error = await self.initialize_internet()
                if not init_success:
                    self._last_recovery_fail_ticks = now_ms
                    self.on_upload_fail()
                    self.is_busy = False
                    return False, 0, f"Initialization failed: {init_error}"
                if not await self.is_healthy():
                    self._last_recovery_fail_ticks = now_ms
                    self.on_upload_fail()
                    self.is_busy = False
                    return False, 0, "Connection failed"

        # --- Periodic signal strength log ---
        if do_health_check:
            signal_pct = await self.get_signal_strength()
            self.save_signal_strength(signal_pct)
            nw = self.get_last_network_type()
            if signal_pct is not None:
                logger.info(
                    f"[CELL] Uploading data, signal: {signal_pct}%, nw_type: {nw}"
                )
            else:
                logger.warning(
                    f"[CELL] Uploading data, signal: unknown, nw_type: {nw}"
                )

        # --- HTTP context: configure once, skip on subsequent calls ---
        if self._last_fail_count >= 2:
            await self._http_stop()
            if not await self._configure_http_context():
                self.on_upload_fail()
                self.is_busy = False
                return False, 0, "Failed to configure HTTP context"

        # --- Convert payload ---
        if isinstance(data, dict):
            import json

            data = json.dumps(data)
        data_length = len(data)

        # --- Step 1: Set URL ---
        for url_attempt in range(2):
            url_length = len(url)
            success, resp = await self._send_command(
                f"AT+QHTTPURL={url_length},80", wait_for="CONNECT", timeout=5
            )
            if success:
                break
            if "711" in resp and url_attempt == 0:
                # PDP context was lost mid-flight - re-activate and retry once.
                print("[CELL] CME ERROR 711: PDP context lost, attempting recovery...")
                await asyncio.sleep(2)
                init_success, init_error = await self.initialize_internet()
                if not init_success:
                    self.on_upload_fail()
                    self.is_busy = False
                    return False, 0, f"CME ERROR 711: PDP recovery failed: {init_error}"
                if not await self._configure_http_context():
                    self.on_upload_fail()
                    self.is_busy = False
                    return False, 0, "CME ERROR 711: HTTP context reconfigure failed"
            else:
                self.on_upload_fail()
                self.is_busy = False
                return False, 0, f"Failed to set URL length: {resp}"

        # Send URL — reduced drain_sleep: URL is short (~40 bytes)
        await self._write_and_drain((url + "\r\n").encode(), drain_sleep=0.1)

        # Wait for OK — reuse fast reader (20 ms poll, 5 s timeout)
        ok_found, response = await self._read_response("OK", timeout_ms=5000)
        if not ok_found:
            self.on_upload_fail()
            self.is_busy = False
            return False, 0, f"Failed to set URL: {response}"

        # --- Step 2: Initiate POST ---
        # No extra settling gap needed — the module is already in HTTP-input mode
        # after the URL OK; adding 300 ms here was pure dead time.
        success, resp = await self._send_command(
            f"AT+QHTTPPOST={data_length},{input_timeout},{response_timeout}",
            wait_for="CONNECT",
            timeout=15,
        )
        if not success:
            self.on_upload_fail()
            self.is_busy = False
            return False, 0, f"Failed to initiate POST: {resp}"

        # --- Step 3: Send body ---
        # Compute a tighter drain based on actual byte count + safety margin.
        # At 115200 baud ≈ 11520 bytes/s → 1 ms per 11.5 bytes.
        # Add 50 ms fixed overhead for module buffering.
        baud_ms = max(50, (data_length * 1000) // 11520 + 50)
        drain_s = baud_ms / 1000.0
        await self._write_and_drain(data.encode(), drain_sleep=drain_s)

        # --- Step 4: Wait for POST response ---
        max_wait_ms = min(response_timeout + 10, 40) * 1000
        start = time.ticks_ms()
        response = ""

        while time.ticks_diff(time.ticks_ms(), start) < max_wait_ms:
            available = getattr(self.uart, "in_waiting", 0)
            if available:
                chunk = self.uart.read(available)
                if chunk:
                    try:
                        response += chunk.decode("utf-8")
                    except Exception:
                        try:
                            response += chunk.decode("latin-1")
                        except Exception:
                            pass

                # Look for +QHTTPPOST: response - only parse once the full line
                # has arrived (newline present) to avoid IndexError on partial reads.
                if (
                    "+QHTTPPOST:" in response
                    and "\n" in response.split("+QHTTPPOST:")[1]
                ):
                    # Queue/EC200 often returns responses:
                    # - +QHTTPPOST: <err>,<http_code>,<content_length>, e.g: +QHTTPPOST: 0,200,106
                    # - +QHTTPPOST: <err>   (some errors return only the first field), e.g: +QHTTPPOST: 702
                    try:
                        raw_after = response.split("+QHTTPPOST:")[1]
                        post_line = raw_after.split("\n")[0].strip()
                        tokens = [t.strip() for t in post_line.split(",")]

                        def _to_int(s, default=0):
                            try:
                                return int(s)
                            except Exception:
                                return default

                        err = _to_int(tokens[0], default=-1) if len(tokens) >= 1 else -1
                        http_code = _to_int(tokens[1], default=0) if len(tokens) >= 2 else 0

                        if err == 0 and http_code == 200:
                            # Step 5: Read response data (shorter read timeout)
                            read_timeout = min(max(response_timeout, 10), 40)
                            success, read_resp = await self._send_command(
                                "AT+QHTTPREAD=80", timeout=read_timeout
                            )
                            self.on_upload_success()
                            self.is_busy = False
                            return True, http_code, read_resp
                        else:
                            self.on_upload_fail()
                            if err == 702:
                                err = f"{err}, Socket Error or HTTP Request Failure"
                            if http_code:
                                self.is_busy = False
                                return (
                                    False,
                                    http_code,
                                    f"HTTP Error: {http_code} (QHTTPPOST err={err})",
                                )
                            self.is_busy = False
                            return False, 0, f"POST failed: QHTTPPOST err={err}"
                    except Exception as e:
                        self.is_busy = False
                        return (
                            False,
                            0,
                            f"Failed to parse response: {str(e)}, Response: {response}",
                        )

                if "ERROR" in response:
                    self.on_upload_fail()
                    self.is_busy = False
                    return False, 0, f"POST failed: {response}"

            await asyncio.sleep(0.02)
            
        self.on_upload_fail()
        self.is_busy = False
        return False, 0, "POST timeout"

    def on_upload_fail(self):
        self._upload_fail_count += 1
        self._last_fail_count += 1
        
    def on_upload_success(self):
        self._upload_success_count += 1
        self._last_fail_count = 0
        
    def get_upload_success_count(self):
        return self._upload_success_count

    def get_upload_fail_count(self):
        return self._upload_fail_count
    
    def get_last_fail_count(self):
        return self._last_fail_count
        
    def get_image_payload(self):
        """Capture a JPEG from the camera, hybrid-encrypt, return API payload dict."""
        try:
            turn_ON_IR_emitter()
            img = sensor.snapshot()
            turn_OFF_IR_emitter()
            jpeg_bytearray = img.compress(quality=25)
            imgbytes = bytes(jpeg_bytearray)
            if uses_hybrid_encryption("P"):
                encnode = enc.EncNode(self.machine_id)
                enc_msgbytes = enc.encrypt_hybrid(imgbytes, encnode.get_pub_key())
                img_b64_str = ubinascii.b2a_base64(enc_msgbytes).rstrip().decode()
            else:
                img_b64_str = ubinascii.b2a_base64(imgbytes).rstrip().decode()
            return {
                "machine_id": self.machine_id,
                "msg_typ": "event",
                "data": img_b64_str,
                "epoch_ms": utime.time_ns() // 1_000_000,
                "enc": ENCRYPTION_ENABLED,
            }
        except Exception as e:
            logger.error(f"[PIR] Failed to get image payload: {e}")
            return None
        finally:
            turn_OFF_IR_emitter()

    def get_heartbeat_payload(self):
        """
        Build heartbeat payload in the same packed format as main.py,
        then base64-encode it for cloud upload.
        Uses sample stats values for driver-level upload testing.
        """
        hbmsg_bytes = build_heartbeat_payload(
            image_taken=14,
            image_sent=5,
            image_dropped=2,
            image_failed=1,
            image_queued=6,
            radio_succ=3923,
            radio_err=48,
            internet_succ=self.get_upload_success_count(),
            internet_err=self.get_upload_fail_count() + 1,
            fs_succ=32,
            fs_err=1,
            neighbours=[215, 216, 217],
            shortest_path=[215, 216, 217],
            process_id=self.process_id,
        )
        hb_b64_str = ubinascii.b2a_base64(hbmsg_bytes).rstrip(b"\n")
        return {
            "machine_id": self.machine_id,
            "msg_typ": "H",
            "epoch_ms": utime.time_ns() // 1_000_000,
            "data": hb_b64_str,
            "enc": False,
        }

    async def make_upload_test(self):
        try:
            url = "https://api.vyomiq.io/watchmen-detect/"
            headers = {
                "Content-Type": "application/json",
                "Authorization": "Bearer YOUR_TOKEN",
            }
            test_count = 5
            ok = 0
            common_img_payload = self.get_image_payload()
            for i in range(1, test_count + 1):
                print(f"uploading {i}/{test_count}.....")
                start_ms = time.ticks_ms()
                success, http_code, response = await self.upload_data(
                    common_img_payload, url, headers=headers
                )
                duration_ms = time.ticks_diff(time.ticks_ms(), start_ms)
                if success:
                    ok += 1
                    print(
                        f"[{i}/{test_count}] SUCCESS | HTTP: {http_code} | {duration_ms/1000:.4f} seconds"
                    )
                    if ok >= 2:
                        print("Upload test passed, returning True...")
                        return True
                else:
                    err = str(response) if response else ""
                    if len(err) > 100:
                        err = err[:100] + "..."
                    print(
                        f"[{i}/{test_count}] FAILED | HTTP: {http_code} | {duration_ms/1000:.4f} seconds | {err}"
                    )
            return ok >= 2
        except Exception as e:
            print(f"Error in make_upload_test: {e}")
            return False

# ------------------------------------------------------------------
# Example Entry point
# ------------------------------------------------------------------

# Global Variables
LOG_DIR = "/sdcard/logs"
MAIN_LOG = LOG_DIR + "/internet_driver.log"
is_writable = False

# Create log directory
def init_dir():
    global is_writable
    try:
        os.mkdir(LOG_DIR)
        is_writable = True
    except OSError:
        is_writable = False
        print("Error: SD card not writable, logs wouldn't be saved.")
        pass
        
def write_log(message):
    if is_writable:
        with open(MAIN_LOG, "a") as f:
            f.write(message + "\n")

if __name__ == "__main__":
    try:
        write_log("STARTING MAIN")
        led_restart_blinker()
        init_dir()
    except Exception as e:
        print(f"Error in logging setup: {e}")
        

    try:
        my_addr = get_my_addr()
        try:
            tracx_uart = UART(UART_ID, BAUDRATE, timeout=2000)
            internet_module = InternetDriver(uart=tracx_uart, configure_sensor=True)
            asyncio.run(internet_module.establish_internet())
        except Exception as e:
            # Keep this handler simple; only treat UARTNotAvailableError specially.
            print(f"Internet driver init failed: {e}, Rebooting...")
            write_log(f"Internet driver init failed: {e}, Rebooting...")
            time.sleep(2)
            machine.reset()

        if not internet_module.initialized:
            print("Internet Module initialization failed! Rebooting...")
            write_log("Internet Module initialization failed!, Rebooting...")
            time.sleep(2)
            machine.reset()

        # Make POST request
        url = "https://api.vyomiq.io/watchmen-detect/"
        payload_1 = internet_module.get_image_payload()
        payload_2 = internet_module.get_heartbeat_payload()
        headers = {
            "Content-Type": "application/json",
            "Authorization": "Bearer YOUR_TOKEN",
        }

        total_uploads = 100
        curr_epoch_ms = utime.time_ns() // 1_000_000
        success_count = 0
        fail_count = 0
        total_duration_ms = 0

        for i in range(total_uploads):
            payload = payload_2 if (i % 2 == 0) else payload_1
            filename = f"{curr_epoch_ms}.json" if (i % 2 == 0) else f"{curr_epoch_ms}.jpg"
            curr_epoch_ms += 1
            payload["epoch_ms"] = curr_epoch_ms
            start_ms = time.ticks_ms()
            success, http_code, response = asyncio.run(
                internet_module.upload_data(payload, url, headers=headers)
            )
            end_ms = time.ticks_ms()
            duration_ms = time.ticks_diff(end_ms, start_ms)
            sec = duration_ms / 1000.0
            total_duration_ms += duration_ms
            attempt_no = i + 1

            if success:
                success_count += 1
                msg_ok = (
                    f"[{attempt_no}/{total_uploads}] SUCCESS | HTTP: {http_code} | "
                    f"{sec:.4f} seconds | {filename}\n"
                )
                print(msg_ok)
                write_log(msg_ok.rstrip())
            else:
                fail_count += 1
                msg_fail = (
                    f"[{attempt_no}/{total_uploads}] FAILED  | HTTP: {http_code} | "
                    f"{sec:.4f} seconds | {filename} | {response}\n"
                )
                print(msg_fail)
                write_log(msg_fail.rstrip())

        avg_duration_ms = (
            (total_duration_ms // total_uploads) if total_uploads > 0 else 0
        )
        print(f"Total: {total_uploads}")
        print(f"Success: {success_count}")
        print(f"Failed: {fail_count}")
        print(f"Average duration: {avg_duration_ms} ms")
        print("END, Rebooting the device...")
        write_log(f"Total: {total_uploads}")
        write_log(f"Success: {success_count}")
        write_log(f"Failed: {fail_count}")
        write_log(f"Average duration: {avg_duration_ms} ms")
        write_log("END, Rebooting the device...")
        time.sleep(2)
        machine.reset()
    except Exception as e:
        print(f"Unexpected error: {e}, Rebooting...")
        write_log(f"Unexpected error: {e}, Rebooting...")
        time.sleep(2)
        machine.reset()

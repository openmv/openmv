import json
import logger
import socket
import time
import ujson
import uasyncio as asyncio
import os
import network
import uselect as select
import ubinascii
import hashlib
WIFI_COMM_PORT_MAP = {
        216: 5001,
        217: 5002,
        218: 5003,
        219: 5004,
        220: 5005,
        221: 5006,
        222: 5007,
        223: 5008,
        224: 5009,
        225: 5010,
        226: 5011,
        227: 5012,
        228: 5013,
        229: 5014,
        230: 5015,
        231: 5016,
        232: 5017,
        233: 5018,
        234: 5019,
        235: 5020,
        236: 5021,
        237: 5022,
        238: 5023,
        239: 5024,
        240: 5025,
        241: 5026,
        242: 5027,
        243: 5028,
        244: 5029,
        245: 5030,
        246: 5031,
        247: 5032,
        248: 5033,
        }
        
# Auto-disconnect WiFi + app TCP session after this many seconds (from successful socket connect).
WIFI_SOCKET_SESSION_TIMEOUT_S = 600
wifi_socket = None
wifi_nic = None
last_connection_attempt_time = 0
recv_timeout = 0.1
apphandler = None
FILE_TRANSFER_BUFFER = None
DATA_BUFFER_SIZE = 4096

_file_transfer_state = None
_message_buffer = ""


class AppController:
    """
    WiFi + app control manager for OpenMV/RT1062.
    This class owns its own socket and transfer state instead of relying on
    module-level globals, which makes it easier to reason about and re‑use.
    """

    def __init__(self, app_handler, my_addr, on_install_mode_exit=None):

        self.apphandler = app_handler
        self.my_addr = my_addr
        self.on_install_mode_exit = on_install_mode_exit
        self.wifi_ssid = "vyom"
        self.wifi_password = "12345678"

        if my_addr not in WIFI_COMM_PORT_MAP:
            logger.error(f"{my_addr} NOT in Port Map Yet")
            self.wifi_comm_port = 0
        else:
            self.wifi_comm_port = WIFI_COMM_PORT_MAP[my_addr]

        self.wifi_socket = None
        self.wifi_nic = None
        self.last_connection_attempt_time = 0
        self.recv_timeout = recv_timeout

        self.DATA_BUFFER_SIZE = DATA_BUFFER_SIZE
        self.file_transfer_buffer = None
        self._file_transfer_state = None

        self._message_buffer = ""
        # If any other code path consumes bytes (e.g. socket liveness checks),
        # we park them here so the main read loop can process them.
        self._recv_buffer = b""
        self.init_file_transfer_buffer()

        self.is_running = False
        self.cont_wifi_fail_count = 0 # continuous wifi failures
        self._wifi_session_deadline = None

    # -------------------------------------------------------------------------
    # WiFi / socket setup
    # -------------------------------------------------------------------------

    def init_file_transfer_buffer(self):
        """Initialize the file transfer buffer at startup when memory is available."""
        try:
            self.file_transfer_buffer = bytearray(self.DATA_BUFFER_SIZE)
            logger.info("[MEM] Pre-allocated file transfer buffer: 2KB")
            return True
        except (MemoryError, Exception) as e:
            logger.error(f"[MEM] Failed to allocate file transfer buffer: {e}")
            self.file_transfer_buffer = None
            return False

    # -------------------------------------------------------------------------
    # High-level control: start/stop/app_alive
    # -------------------------------------------------------------------------

    async def start(self):
        """
        Start WiFi + app communication:
        - Initialize WiFi (STA) if debugging is enabled.
        - Start background monitor task to keep connection alive.
        - Start socket read loop task.
        """
        self.is_running = True
        # self._monitor_wifi_connection()
        # self.wifi_socket_read_loop()
        # asyncio.create_task(self._monitor_wifi_connection())
        # asyncio.create_task(self.wifi_socket_read_loop())
        loop = asyncio.get_event_loop()
        loop.create_task(self._monitor_wifi_connection())
        loop.create_task(self.wifi_socket_read_loop())
        # loop.create_task(self._periodic_log_sender())


    async def stop(self):
        """
        Stop WiFi + app communication:
        - Signal background loops to exit.
        - Close socket and disable WiFi.
        """
        try:
            print(f"stopping app controller")
            self.on_install_mode_exit()
            self.wifi_socket.close()
            self.wifi_nic.disconnect()
            self.wifi_nic.active(False)
        except Exception as e:
            print(f"Error closing socket: {e}")
            pass
        self.wifi_socket = None
        self.wifi_nic = None
        self._wifi_session_deadline = None
        self.is_running = False

    def app_alive(self):
        """
        Simple liveness check used by install mode:
        consider the app "alive" while WiFi comms are enabled and
        we still have an open socket.
        """
        if not self.is_running:
            logger.info("[APP] App not running, self.is_running = False")
            return False
        if not self.wifi_nic.isconnected():
            logger.info("[APP] WiFi not connected, self.wifi_nic.isconnected() = False")
            return False
        if self.cont_wifi_fail_count >= 3:
            logger.info("[APP] Continuous wifi failures, self.cont_wifi_fail_count = {self.cont_wifi_fail_count}")
            return False
        return True

    # -------------------------------------------------------------------------
    # WiFi / socket setup internals
    # ------------------------------------------------------------------------

    async def _init_wifi(self):
        """
        Initialize WiFi NIC and connect to the configured SSID.
        Returns True on success, False otherwise.
        """
        try:
            self.wifi_nic = network.WLAN(network.WLAN.IF_STA)
            try:
                self.wifi_nic.active(False)
                await asyncio.sleep(1)
            except Exception:
                pass

            self.wifi_nic.active(True)
            await asyncio.sleep(1)

            if self.wifi_nic.isconnected():
                logger.info("[WIFI] Already connected")
                return True

            logger.info(f"[WIFI] Connecting to {self.wifi_ssid}")
            self.wifi_nic.connect(self.wifi_ssid, self.wifi_password)

            timeout = 3
            for i in range(timeout):
                if self.wifi_nic.isconnected():
                    ip = self.wifi_nic.ifconfig()[0]
                    self._init_socket_connection()
                    logger.info(f"[WIFI] Connected, IP: {ip}")
                    return True

                logger.info(
                    f"[WIFI] Waiting for connection... {i+1}s status: {self.wifi_nic.status()}"
                )
                await asyncio.sleep(5)

            logger.error("[WIFI] Connection timeout")
            self.wifi_nic.disconnect()
            self.wifi_nic.active(False)
            return False

        except Exception as e:
            logger.error(f"[WIFI] init error: {e}")
            
            if self.wifi_nic is not None:
                try:
                    self.wifi_nic.disconnect()
                    self.wifi_nic.active(False)
                except Exception:
                    pass
            return False

    def _close_socket_safely(self, sock):
        try:
            sock.shutdown(socket.SHUT_RDWR)
        except:
            pass
        if sock == self.wifi_socket:
            self.wifi_socket = None
            self._wifi_session_deadline = None

    def send_data_to_app(self, data, timeout=0.1):
        if self.wifi_socket is None:
            return False, None

        if isinstance(data, dict):
            data = ujson.dumps(data).encode("utf-8")
        elif isinstance(data, str):
            data = data.encode("utf-8")

        try:
            # Set timeout for this operation
            self.wifi_socket.settimeout(timeout)
            self.wifi_socket.sendall(data)
            return True, self.wifi_socket
        except OSError as e:
            errno_val = getattr(e, "errno", None)
            if errno_val in (104, 107, 116):
                print(f"Connection error during send: {e}")
                self._close_socket_safely(self.wifi_socket)
                return False, None
            else:
                print(f"OSError during send: {e}")
                return True, self.wifi_socket
        except Exception as e:
            error_str = str(e)
            if "timeout" in error_str.lower() or "lwip" in error_str.lower():
                print(f"Non-fatal error (ignoring): {e}")
                return True, self.wifi_socket
            else:
                print(f"Unexpected error: {e}")
                self._close_socket_safely(self.wifi_socket)
                return False, None
    def check_connection(self):
        wifi_connected = self.wifi_nic is not None and self.wifi_nic.isconnected()
        socket_connected = self.wifi_socket is not None and self._is_socket_alive()
        return {"wifi_connected": wifi_connected, "socket_connected": socket_connected}
        

    def check_wifi_connection_status(self):
        """
        Check WiFi connection status and socket health.
        Updates internal flags: wifi_socket.
        """
        if self.wifi_nic is None:
            if self.wifi_socket is not None:
                try:
                    self.wifi_socket.close()
                except:
                    pass
                self.wifi_socket = None
                self._wifi_session_deadline = None
            return

        is_connected = self.wifi_nic.isconnected()

        if not is_connected and self.wifi_socket is not None:
            print("WARNING - WiFi disconnected, disabling WiFi communication")
            try:
                self.wifi_socket.close()
            except:
                pass
            self.wifi_socket = None
            self._wifi_session_deadline = None

    async def _monitor_wifi_connection(self):
        """
        Periodic task to monitor WiFi connection status and keep app socket alive.
        This is the class-based equivalent of main.monitogit_wifi_connection().
        """
        
        connected_check_interval = 10
        http_reconnect_interval = 10
        disconnected_check_interval = 3
        error_backoff_interval = 10

        while self.is_running:
            try:
                self.check_wifi_connection_status()
                is_connected = self.wifi_nic is not None and self.wifi_nic.isconnected()

                if is_connected:
                    logger.debug("[WIFI] WiFi connection status: connected")

                    
                    if self.wifi_socket is not None and self._is_socket_alive():
                        self.cont_wifi_fail_count = 0
                        # if (
                        #     self._wifi_session_deadline is not None
                        #     and time.time() >= self._wifi_session_deadline
                        # ):
                        #     await self._wifi_session_timeout_disconnect()
                        #     continue
                    else:
                        self._init_socket_connection()

                    await asyncio.sleep(http_reconnect_interval)
                else:
                    logger.info("[WIFI] WiFi not connected, attempting to reconnect...")
                    success = await self._init_wifi()
                    if success:
                        self.cont_wifi_fail_count = 0
                        logger.info("[WIFI] WiFi reconnection successful!")
                        await asyncio.sleep(connected_check_interval)
                    else:
                        self.cont_wifi_fail_count += 1
                        logger.warning(
                            "[WIFI] WiFi reconnection failed, "
                            f"will retry in {disconnected_check_interval} seconds"
                        )
                        await asyncio.sleep(disconnected_check_interval)
            except Exception as e:
                self.cont_wifi_fail_count += 1
                logger.error(f"[WIFI] Error in WiFi connection monitoring: {e}")
                await asyncio.sleep(error_backoff_interval)

    def _is_socket_alive(self):
        try:
            if select.select([self.wifi_socket], [], [], 0)[0]:
                return True
            return True
        except Exception as e:
            print(f"[WIFI] Socket alive check error: {e}")
            return True
    
    def _init_socket_connection(self, max_retries=3):
        """
        Initialize or re-initialize the WiFi socket connection to the hotspot server.
        """
        if self.wifi_nic is None or not self.wifi_nic.isconnected():
            print("WARNING - WiFi not connected, cannot initialize WiFi communication")
            if self.wifi_nic:
                self.wifi_nic.close()
            self.wifi_socket = None
            self._wifi_session_deadline = None
            return False

        if self.wifi_socket is not None:
            self.wifi_socket = None
            self._wifi_session_deadline = None

        try:
            self.last_connection_attempt_time = time.time()
            ifconfig = self.wifi_nic.ifconfig()
            device_ip, _, gateway_ip = ifconfig[0], ifconfig[1], ifconfig[2]
            print(f"[WIFI] Device IP: {device_ip}, Gateway IP: {gateway_ip}")
            target_ip = gateway_ip

            new_socket = None
            for attempt in range(max_retries):
                try:
                    if not self.wifi_nic.isconnected():
                        print(
                            "WARNING - WiFi disconnected during socket init, "
                            "skipping further server connection attempts"
                        )
                        break

                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.settimeout(3)

                    print(
                        f"Connecting to {target_ip}:{self.wifi_comm_port}"
                        f" (attempt {attempt + 1}/{max_retries})"
                    )
                    sock.connect((target_ip, self.wifi_comm_port))
                    new_socket = sock
                    break
                except Exception as e:
                    print(f"Connection attempt {attempt + 1} failed: {e}")
                    if attempt < max_retries - 1:
                        asyncio.sleep(1)
                    else:
                        self.cont_wifi_fail_count += 1
                        print(
                            "All connection attempts failed. "
                            "Will retry in monitoring loop."
                        )

            if new_socket is not None:
                self.wifi_socket = new_socket
                self._wifi_session_deadline = time.time() + WIFI_SOCKET_SESSION_TIMEOUT_S
                print(
                    f"info - WiFi communication enabled, "
                    f"connected to {target_ip}:{self.wifi_comm_port}"
                )
                self.cont_wifi_fail_count = 0
                self.update_hbstatus()
                return True

            self.wifi_socket = None
            self._wifi_session_deadline = None
            return False
        except Exception as e:
            print(f"ERROR - Failed to initialize WiFi communication: {e}")
            self.wifi_socket = None
            self._wifi_session_deadline = None
            return False

    async def _wifi_session_timeout_disconnect(self):
        """Close WiFi + app socket after WIFI_SOCKET_SESSION_TIMEOUT_S from connect."""
        logger.info("[WIFI] App session timed out; disconnecting WiFi and socket")
        self._wifi_session_deadline = None
        try:
            if self.wifi_socket is not None:
                self.create_and_send_message("disconnect", "session_timeout")
                await asyncio.sleep(0.3)
        except Exception:
            pass
        await self.stop()

    # -------------------------------------------------------------------------
    # Socket read loop
    # -------------------------------------------------------------------------

    async def wifi_socket_read_loop(self):
        print("info - Starting WiFi socket read loop")        
        consecutive_empty_reads = 0

        while self.is_running:
            if self.wifi_socket is None:
                await asyncio.sleep(3)
                consecutive_empty_reads = 0
                continue

            try:
                self.wifi_socket.settimeout(self.recv_timeout)
                data = self.wifi_socket.recv(1024)

                # If some other path previously consumed bytes, prepend them.
                if getattr(self, "_recv_buffer", None):
                    data = self._recv_buffer + (data or b"")
                    self._recv_buffer = b""

                if data and len(data) > 0:
                    self.recv_timeout = 0.1
                    consecutive_empty_reads = 0

                    try:
                        data_str = data.decode("utf-8")
                        # print(f"[WIFI_READ] Received {len(data_str)} bytes")

                        self._message_buffer += data_str

                        while self._message_buffer:
                            self._message_buffer = self._message_buffer.lstrip()

                            if not self._message_buffer:
                                break

                            try:
                                message = ujson.loads(self._message_buffer)
                                self.handle_message(message)
                                self._message_buffer = ""
                                break
                            except ValueError:
                                newline_pos = self._message_buffer.find("\n")
                                if newline_pos != -1:
                                    potential_message = self._message_buffer[:newline_pos]
                                    try:
                                        message = ujson.loads(potential_message)
                                        print(
                                            "[WIFI_READ] Processing message: "
                                            f"{message.get('message_type', 'unknown')}"
                                        )
                                        self.handle_message(message)
                                        self._message_buffer = self._message_buffer[newline_pos + 1:]
                                        continue
                                    except:
                                        pass

                                brace_count = 0
                                in_string = False
                                escape_next = False

                                for i, char in enumerate(self._message_buffer):
                                    if escape_next:
                                        escape_next = False
                                        continue
                                    if char == "\\":
                                        escape_next = True
                                        continue
                                    if char == '"' and not escape_next:
                                        in_string = not in_string
                                    if not in_string:
                                        if char == "{":
                                            brace_count += 1
                                        elif char == "}":
                                            brace_count -= 1
                                            if brace_count == 0:
                                                potential_message = self._message_buffer[: i + 1]
                                                try:
                                                    message = ujson.loads(potential_message)
                                                    print(
                                                        "[WIFI_READ] Processing message: "
                                                        f"{message.get('message_type', 'unknown')}"
                                                    )
                                                    self.handle_message(message)
                                                    self._message_buffer = self._message_buffer[i + 1:]
                                                    break
                                                except:
                                                    pass
                                else:
                                    break

                    except Exception as e:
                        print(f"[WIFI_READ] Error processing data: {e}")
                        self._message_buffer = ""

                    await asyncio.sleep(0.1)
                else:
                    self.recv_timeout = 0.1
                    consecutive_empty_reads += 1

                    if consecutive_empty_reads > 10:
                        await asyncio.sleep(5)
                    else:
                        await asyncio.sleep(0.05)

            except OSError as e:
                errno_val = getattr(e, "errno", None)
                if errno_val == 116:
                    self.recv_timeout = 0.1
                    await asyncio.sleep(0.1)
                elif errno_val in (104, 107):
                    print(f"[WIFI_READ] Connection lost: {e}")
                    self._close_socket_safely(self.wifi_socket)
                    self._message_buffer = ""
                    self.recv_timeout = 0.1
                    consecutive_empty_reads = 0
                    await asyncio.sleep(3)
                else:
                    print(f"[WIFI_READ] Socket error: {e}")
                    self.recv_timeout = 0.1
                    await asyncio.sleep(0.5)
            except Exception as e:
                print(f"[WIFI_READ] Unexpected error: {e}")
                self.recv_timeout = 0.1
                await asyncio.sleep(0.5)

    # async def _periodic_log_sender(self):   
    #     while self.is_running:
    #         try:
    #             # print('++++++++++++++ sending logs ++++++++++++++')
    #             if self.wifi_socket is not None and self.wifi_nic and self.wifi_nic.isconnected():
    #                 logs = logger.return_saved_logs_and_clear()
    #                 if logs:
    #                     for log in logs:
    #                         self.create_and_send_message("log", log, timeout=0.5)
                       
    #         except Exception as e:
    #             logger.error(f"[PERIODIC_LOGS] Error in periodic log sender: {e}")
            
    #         await asyncio.sleep(3)

    # -------------------------------------------------------------------------
    # File receive helpers
    # -------------------------------------------------------------------------

    def _get_file_save_root(self):
        """Return root path for saving received files (sdcard or flash)."""
        try:
            os.listdir("/sdcard")
            return "/sdcard"
        except OSError:
            return "/flash"

    def _handle_start_file_transfer(self, message):
        file_name = message.get("data")
        payload = message.get("payload", {})
        no_of_chunks = payload.get("no_of_chunks", 0)

        if not file_name:
            logger.error("[FILE_RECV] start_file_transfer missing file_name")
            return

        if self._file_transfer_state and self._file_transfer_state.get("file_handle"):
            try:
                self._file_transfer_state["file_handle"].close()
            except Exception:
                pass

        root = self._get_file_save_root()
        save_path = f"{root}/{file_name}"

        try:
            f = open(save_path, "wb")
            self._file_transfer_state = {
                "file_name": file_name,
                "save_path": save_path,
                "file_handle": f,
                "expected_chunks": no_of_chunks,
                "received_chunks": 0,
                "chunks": {},
            }
            logger.info(
                f"[FILE_RECV] Started receiving file: {file_name} "
                f"({no_of_chunks} chunks expected)"
            )
        except Exception as e:
            logger.error(f"[FILE_RECV] Failed to open file for write: {e}")
            self._file_transfer_state = None

    def _handle_file_chunk(self, message):
        if self._file_transfer_state is None:
            logger.error("[FILE_RECV] file_chunk received without start_file_transfer")
            return

        data_b64 = message.get("data")
        chunk_index = message.get("payload")

        if data_b64 is None or chunk_index is None:
            logger.error("[FILE_RECV] file_chunk missing data or chunk_index")
            return

        try:
            chunk_bytes = ubinascii.a2b_base64(data_b64)

            self._file_transfer_state["chunks"][chunk_index] = chunk_bytes
            self._file_transfer_state["received_chunks"] += 1

            print(
                f"[FILE_RECV] Received chunk {chunk_index} ({len(chunk_bytes)} bytes) - "
                f"{self._file_transfer_state['received_chunks']}/"
                f"{self._file_transfer_state['expected_chunks']}"
            )
        except Exception as e:
            logger.error(f"[FILE_RECV] Failed to decode chunk {chunk_index}: {e}")

    def _handle_end_file_transfer(self):
        if self._file_transfer_state is None:
            logger.error("[FILE_RECV] end_file_transfer received without active transfer")
            return

        f = self._file_transfer_state.get("file_handle")
        save_path = self._file_transfer_state.get("save_path", "?")
        file_name = self._file_transfer_state.get("file_name", "?")
        expected_chunks = self._file_transfer_state.get("expected_chunks", 0)
        received_chunks = self._file_transfer_state.get("received_chunks", 0)
        chunks = self._file_transfer_state.get("chunks", {})

        try:
            if f:
                for i in range(expected_chunks):
                    if i in chunks:
                        f.write(chunks[i])
                    else:
                        logger.error(f"[FILE_RECV] Missing chunk {i}")

                f.flush()
                f.close()
                try:
                    os.sync()
                except (OSError, AttributeError):
                    pass

            if received_chunks == expected_chunks:
                logger.info(
                    f"[FILE_RECV] Successfully saved file: {save_path} "
                    f"({received_chunks} chunks)"
                )
                self.send_data_to_app(
                    {
                        "message_type": "end_file_transfer",
                        "data:": {
                            "file_name": file_name,
                            "save_path": save_path,
                            "chunks_received": received_chunks,
                            "chunks_expected": expected_chunks,
                        },
                        "timestamp": time.time(),
                    }
                )
                import machine

                self.recv_timeout = 0.1
                self._file_transfer_state = None
                logger.error("==== Rebooting the device in 10 seconds ==== ")
                asyncio.sleep(10)
                try:
                    os.sync()
                except (OSError, AttributeError):
                    pass
                asyncio.sleep(0.5)
                machine.reset()
            else:
                logger.error(
                    "[FILE_RECV] File incomplete: "
                    f"{received_chunks}/{expected_chunks} chunks received"
                )
                self.send_data_to_app(
                    {
                        "message_type": "end_file_transfer",
                        "data:": {
                            "file_name": file_name,
                            "save_path": save_path,
                            "chunks_received": received_chunks,
                            "chunks_expected": expected_chunks,
                        },
                        "timestamp": time.time(),
                    }
                )
                self.recv_timeout = 0.1
                self._file_transfer_state = None
        except Exception as e:
            logger.error(f"[FILE_RECV] Failed to close/save file: {e}")

    # -------------------------------------------------------------------------
    # File / log send helpers
    # -------------------------------------------------------------------------

    def send_log_file(self, filename="main.log"):
        """
        Stream the main log file over the WiFi socket in chunks.
        This is triggered by the 'download_logs' command.
        Uses the same path as the logger so we read from where logs are written.
        """
        log_path = None
        try:
            try:
                os.listdir("/sdcard")
                FS_ROOT = "/sdcard"
            except OSError:
                FS_ROOT = "/flash"

            log_path = f"{FS_ROOT}/logs/{filename}"

            if not log_path:
                logger.error("Log path not available, cannot send logs")
                return

            start_msg = {
                "message_type": "log_file_start",
                "file_name": "main.log",
                "timestamp": time.time(),
            }
            ok, _ = self.send_data_to_app(start_msg, 0.5)
            if not ok:
                print("[WIFI] Failed to send log_file_start")
                return

            CHUNK_SIZE = 2048
            chunk_index = 0

            with open(log_path, "r") as f:
                while True:
                    chunk = f.read(CHUNK_SIZE)
                    if not chunk:
                        break

                    chunk_msg = {
                        "message_type": "log_file_chunk",
                        "file_name": "main.log",
                        "chunk_index": chunk_index,
                        "data": chunk,
                    }

                    ok, _ = self.send_data_to_app(chunk_msg, 5)
                    if not ok:
                        print(f"[WIFI] Failed to send log_file_chunk {chunk_index}")
                        return

                    chunk_index += 1

            end_msg = {
                "message_type": "log_file_end",
                "file_name": "main.log",
                "total_chunks": chunk_index,
                "timestamp": time.time(),
            }
            self.send_data_to_app(end_msg, 0.5)
        except Exception as e:
            logger.error(f"[WIFI] Failed to stream log file '{log_path}': {e}")

    # -------------------------------------------------------------------------
    # Heartbeat / status
    # -------------------------------------------------------------------------

    def update_hbstatus(self):
        data = self.apphandler.send_hb_data()
        parts = data.split(":")

        def _part(i, default=""):
            return parts[i].strip() if i < len(parts) else default

        disarmed_val = _part(7).lower() == "true"
        heartbeat_dict = {
            "my_addr": _part(0),
            "epoch_sec": _part(1),
            "total_image_count": _part(2),
            "gps_coords": _part(3),
            "gps_staleness": _part(4),
            "seen_neighbours": _part(5),
            "shortest_path_to_cc": _part(6),
            "disarmed": disarmed_val,
        }
        msg = {
            "message_type": "heartbeat",
            "data": heartbeat_dict,
            "timestamp": time.time(),
        }
        self.send_data_to_app(msg)

    def report_radio_check_result(self, success_count, success_rate, transfer_rate):
        msg = {
            "message_type": "radio_check",
            "data": 
                {
                    "success_count": success_count,
                    "success_rate_percent": success_rate,
                    "transfer_rate_msgs_per_sec": transfer_rate,
                }
            ,
            "timestamp": time.time(),
        }
        self.send_data_to_app(msg)

    def radio_check(self, target_addr=0, byte_count=30):
        print(f"Checking radio connectivity with {target_addr} with byte count {byte_count}")
        self.create_and_send_message("radio_check", {"message": f"Checking radio connectivity with {target_addr}"}, timeout=0.5)
        async def _run_check():
            result = await self.apphandler.check_radio_connectivity_with(
                target_addr, 10, byte_count
            )
            self.report_radio_check_result(result[0], result[1], result[2])

        try:
            loop = asyncio.get_event_loop()
            loop.create_task(_run_check())
        except Exception as e:
            logger.error(f"radio_check failed to schedule task: {e}")

    # -------------------------------------------------------------------------
    # Image / alert queues
    # -------------------------------------------------------------------------

    def list_images(self):
        imagelist = self.apphandler.list_images()
        self.create_and_send_message("list_images", imagelist, 5)

    def clear_image_queue(self):
        return self.apphandler.clear_image_queue()

    def clear_all_queue(self):
        return self.apphandler.clear_all_queue()

    def get_image(self, imagename):
        imgfile = "/sdcard/" + imagename
        self.send_log_file(imgfile)

    def send_image_in_chunks(self, image_path):
        if self.file_transfer_buffer is None:
            logger.error("[IMAGE_TRANSFER] FILE_TRANSFER_BUFFER is not initialized")
            return False

        try:
            filename = image_path.split("/")[-1]

            md5_hash = hashlib.md5()
            with open(image_path, "rb") as f:
                while True:
                    bytes_read = f.readinto(self.file_transfer_buffer)
                    if bytes_read == 0:
                        break
                    md5_hash.update(self.file_transfer_buffer[:bytes_read])
            img_md5 = ubinascii.hexlify(md5_hash.digest()).decode()
            logger.info(f"[IMAGE_TRANSFER] Sending {filename} (md5={img_md5})")

            start_msg_data = {
                "file_name": filename,
                "file_path": image_path,
                "md5": img_md5,
            }
            self.create_and_send_message("image_transfer_start", start_msg_data, timeout=1.0)

            chunk_index = 0
            with open(image_path, "rb") as f:
                while True:
                    bytes_read = f.readinto(self.file_transfer_buffer)

                    if bytes_read == 0:
                        break

                    chunk_data = bytes(self.file_transfer_buffer[:bytes_read])
                    chunk_b64_str = ubinascii.b2a_base64(chunk_data).decode("utf-8").strip()

                    chunk_msg_data = {
                        "file_name": filename,
                        "chunk_index": chunk_index,
                        "chunk_size": bytes_read,
                        "data": chunk_b64_str,
                    }

                    self.create_and_send_message("image_transfer_chunk", chunk_msg_data, timeout=5.0)
                    logger.info(
                        f"[IMAGE_TRANSFER] Sent chunk {chunk_index} ({bytes_read} bytes)"
                    )

                    chunk_index += 1

            end_msg_data = {
                "file_name": filename,
                "file_path": image_path,
                "total_chunks": chunk_index,
                "md5": img_md5,
            }
            self.create_and_send_message("image_transfer_end", end_msg_data, timeout=1.0)

            return True

        except (OSError, MemoryError, Exception) as e:
            error_msg = f"Transfer failed: {str(e)}"
            if isinstance(e, OSError):
                error_msg = f"File error: {str(e)}"
            elif isinstance(e, MemoryError):
                error_msg = f"Memory error: {str(e)}"
            
            self.create_and_send_message("image_transfer_error", error_msg, timeout=1.0)
            return False

    # -------------------------------------------------------------------------
    # verify_internet (CC only: capture image, upload to server, inform app)
    # -------------------------------------------------------------------------

    async def _handle_verify_internet(self, force_upload=False): # if force_upload = True and this unit node,, first call internet_module.establish_internet(retry_count=2), then rest if same
        is_cc = self.apphandler.is_cc()
        if not is_cc:
            if force_upload:
                ok = await self.apphandler.try_create_cc()
                if ok:
                    self.create_and_send_message("verify_internet", {"message": "CC created successfully", "result": "pass"}, timeout=0.5)
                else:
                    self.create_and_send_message("verify_internet", {"message": "Failed to create CC", "result": "fail"}, timeout=0.5)
                    return False
            else:
                self.create_and_send_message("verify_internet", {"message": "checking internet connection"}, timeout=0.5)
                self.create_and_send_message("verify_internet", {"message": "not running as CC", "result": "fail"}, timeout=0.5)
                return False
        try:
            self.create_and_send_message("verify_internet", {"message": "checking internet connection"}, timeout=0.5)
            run_fn = getattr(self.apphandler, "verify_internet_capture_and_upload", None)
            if not run_fn:
                self.create_and_send_message("verify_internet", {"message": "verify_internet capture+upload not available", "result": "fail"}, timeout=0.5)
                return
            try:
                s = time.ticks_ms()
                ok = await run_fn()
                upload_duration = max(time.ticks_diff(time.ticks_ms(), s) / 1000.0, 1e-6)
                if ok:
                    self.create_and_send_message("verify_internet", {"message": f"upload succeeded in {upload_duration:.3f} seconds", "result": "pass"}, timeout=0.5)
                else:
                    self.create_and_send_message("verify_internet", {"message": f"upload failed after {upload_duration:.3f} seconds", "result": "fail"}, timeout=0.5)
            except Exception as e:
                self.create_and_send_message("verify_internet", {"message": f"upload error: {e}", "result": "fail"}, timeout=0.5)
        finally:
            pass

    async def handle_check_network(self):
        try:
            self.create_and_send_message("check_network", {"message": "Running network scan"}, timeout=0.5)
            check_network_result = await self.apphandler.check_network()
            if not check_network_result:
                self.create_and_send_message("check_network", {"message": "Network scan failed", "result": "fail"}, timeout=0.5)
        except Exception as e:
            self.create_and_send_message("check_network", {"message": f"Network scan failed: {e}", "result": "fail"}, timeout=0.5)
    # -------------------------------------------------------------------------
    # Message / command handling
    # -------------------------------------------------------------------------

    def forward_log_line(self, line):
        if self.wifi_socket is None:
            return
        try:
            self.create_and_send_message("log", line, timeout=0.5)
        except Exception:
            pass

    def get_recent_logs(self):
        all_logs = self.apphandler.get_saved_logs()
        alength = len(all_logs)
        start = alength - 10
        start = 0 if start < 0 else start
        for i in range(start, alength):
            self.create_and_send_message("log", all_logs[i], timeout=0.5)

    def handle_message(self, message):
        msg_type = message.get("message_type")
        if msg_type == "command":
            self.handle_command(message)
        elif msg_type == "start_file_transfer":
            self.recv_timeout = 5.0
            self._handle_start_file_transfer(message)
            self.create_and_send_message("ack", "start_file_transfer")
        elif msg_type == "file_chunk":
            self.recv_timeout = 5.0
            self._handle_file_chunk(message)
        elif msg_type == "end_file_transfer":
            self._handle_end_file_transfer()

    async def _exit_install_mode_from_command(self):
        """Gracefully exit install mode when app sends exit command."""
        await asyncio.sleep(1)
        await self.stop()

    def handle_command(self, message):
        command = message.get("data")
        if command == "show_status":
            self.update_hbstatus()
            self.recv_timeout = 2.0
        elif command == "verify_internet":
            logger.info(f"received command: {message}")
            try:
                asyncio.create_task(self._handle_verify_internet())
            except Exception as e:
                logger.error(f"[verify_internet] Failed to create task: {e}")
        elif command == "try_create_cc":
            logger.info(f"received command: {message}")
            try:
                asyncio.create_task(self._handle_verify_internet(force_upload=True))
            except Exception as e:
                logger.error(f"[try_create_cc] Failed to create task: {e}")
        elif command == "check_network":
            logger.info(f"received command: {message}")
            asyncio.create_task(self.handle_check_network())
        elif command == "radio_check":
            logger.info(f"received command: {message}")
            payload = message.get("payload")
            self.radio_check(
                int(payload.get("target_addr", 0)),
                int(payload.get("byte_count", 30)),
            )
        elif command == "verify_image":
            logger.info(f"received command: {message}")
            try:
                asyncio.create_task(self.apphandler.send_image_to_app())
            except Exception as e:
                logger.error(f"[VERIFY_IMAGE] Failed to create task: {e}")
        elif command == "reboot":
            logger.info(f"received command: {message}")
            import machine
            logger.error("==== Rebooted by APP --- will reboot in 10 seconds ==== ")
            self._close_socket_safely(self.wifi_socket)
            self.wifi_socket = None
            self.create_and_send_message("disconnect", "reboot")
            asyncio.sleep(10)
            machine.reset()
#================================================ unused ================================================
        elif command == "set_disarmed":
            logger.info(f"received command: {message}")
            payload = message.get("payload")
            if payload and payload == "arm":
                self.apphandler.arm()
                self.update_hbstatus()
            elif payload and payload == "disarm":
                self.apphandler.disarm()
                self.update_hbstatus()
            else:
                logger.error("invalid value provided")
        elif command == "get_logs":
            self.get_recent_logs()
        elif command == "list_images":
            self.list_images()
        elif command == "clear_image_queue":
            self.clear_image_queue()
        elif command == "download_image":
            logger.info(f"received command: {message}")
            image_path = message.get("payload")
            if image_path:
                self.send_image_in_chunks(image_path)
            else:
                logger.error("no image path provided")
        elif command == "clear_all_queue":
            self.clear_all_queue()
        elif command == "exit_install_mode":
            logger.info("received command: exit_install_mode")
            self.create_and_send_message("exit_install_mode_ack", "exit_install_mode", timeout=1.0)
            asyncio.create_task(self._exit_install_mode_from_command())
        else:
            logger.info(f"Unknown command: {command}")

    def create_and_send_message(self, message_type, data, timeout=0.5):
        msg = {
            "message_type": message_type,
            "data": data,
            "timestamp": time.time(),
        }
        self.send_data_to_app(msg, timeout)
    
    @staticmethod
    def _extract_complete_messages(buffer):
        """
        Extract complete JSON messages from buffer.
        Messages are newline-delimited.
        Returns: (list of complete messages, remaining buffer)
        """
        messages = []
        lines = buffer.split("\n")

        for line in lines[:-1]:
            line = line.strip()
            if line:
                try:
                    msg = ujson.loads(line)
                    messages.append(msg)
                except Exception as e:
                    print(f"[WIFI_READ] Failed to parse JSON line: {e}")
                    print(f"[WIFI_READ] Problematic line: {line[:100]}...")

        return messages, lines[-1]

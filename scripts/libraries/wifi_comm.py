import json
import logger
import socket
import time
import ujson
import uasyncio as asyncio
import os

WIFI_COMM_PORT_MAP = {
    219: 5001,
    221: 5002,
    222: 5003,
    223: 5004,
    224: 5005,
    225: 5006,
}
WIFI_COMM_PORT = 0
communication = False
wifi_comm_enabled = False
wifi_socket = None
wifi_nic = None
wifi_logging_enabled = True
last_connection_attempt_time = 0
CONNECTION_RETRY_COOLDOWN = 30
recv_timeout = 0.1
apphandler = None
myaddr = 0
FILE_TRANSFER_BUFFER = None
DATA_BUFFER_SIZE = 2048

# State for receiving file transfers from Android
_file_transfer_state = None

# Message buffering - crucial for handling fragmented JSON
_message_buffer = ""


def init_wifi_comm(ah, myad):
    global apphandler
    apphandler = ah
    global myaddr
    myaddr = myad
    global WIFI_COMM_PORT
    if myaddr not in WIFI_COMM_PORT_MAP:
        logger.error(f"{myaddr} NOT in Port Map Yet")
    WIFI_COMM_PORT = WIFI_COMM_PORT_MAP[myaddr]


def init_file_transfer_buffer():
    """Initialize the global file transfer buffer at startup when memory is available"""
    global FILE_TRANSFER_BUFFER
    try:
        FILE_TRANSFER_BUFFER = bytearray(DATA_BUFFER_SIZE)
        logger.info("[MEM] Pre-allocated file transfer buffer: 2KB")
        return True
    except MemoryError as e:
        logger.error(f"[MEM] Failed to allocate file transfer buffer: {e}")
        FILE_TRANSFER_BUFFER = None
        return False
    except Exception as e:
        logger.error(f"[MEM] Error allocating file transfer buffer: {e}")
        FILE_TRANSFER_BUFFER = None
        return False


def create_persistent_connection(wifi_interface, host, port, max_retries=3):
    global wifi_socket
    if wifi_socket is not None:
        _close_socket_safely(wifi_socket)
        wifi_socket = None
    for attempt in range(max_retries):
        try:
            if not wifi_interface.isconnected():
                print("WARNING - WiFi not connected, skipping server connection attempts")
                return None
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(10)

            print(f"Connecting to {host}:{port}... (attempt {attempt + 1}/{max_retries})")
            sock.connect((host, port))
            print("Connected! Connection established and kept alive.")
            try:
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
            except AttributeError:
                pass
            return sock
        except Exception as e:
            print(f"Connection attempt {attempt + 1} failed: {e}")
            if attempt < max_retries - 1:
                time.sleep(1)
            else:
                print("All connection attempts failed. Will retry in main loop.")
                return None
    return None


def _close_socket_safely(sock):
    global wifi_socket
    try:
        sock.shutdown(socket.SHUT_RDWR)
    except Exception:
        pass
    if sock == wifi_socket:
        wifi_socket = None


def send_data_to_app(data, timeout=0.1):
    global communication

    try:
        if wifi_socket is None:
            communication = False
            return False, None

        if isinstance(data, dict):
            data = ujson.dumps(data).encode('utf-8')
        elif isinstance(data, str):
            data = data.encode('utf-8')

        try:
            old_timeout = wifi_socket.gettimeout()
        except Exception:
            old_timeout = None
        try:
            wifi_socket.settimeout(timeout)
            wifi_socket.sendall(data)
        finally:
            try:
                if old_timeout is not None:
                    wifi_socket.settimeout(old_timeout)
            except Exception:
                pass
        # print(f"Sent message: {len(data)} bytes")
        communication = True
        return True, wifi_socket

    except OSError as e:
        errno = getattr(e, 'errno', None)
        if errno in (104, 107, 116):
            print(f"Connection error during send: {e}")
            communication = False
            _close_socket_safely(wifi_socket)
            return False, None
        else:
            print(f"OSError during send: {e}")
            communication = True
            return True, wifi_socket

    except Exception as e:
        error_str = str(e)
        if "timeout" in error_str.lower() or "lwip" in error_str.lower():
            print(f"Non-fatal error (ignoring): {e}")
            communication = True
            return True, wifi_socket
        else:
            print(f"Unexpected error: {e}")
            communication = False
            _close_socket_safely(wifi_socket)
            return False, None


def create_wifi_connection(wifi_interface, max_retries=3):
    if wifi_interface is None or not wifi_interface.isconnected():
        print("WARNING - WiFi interface not connected")
        return None

    try:
        ifconfig = wifi_interface.ifconfig()
        device_ip = ifconfig[0]
        gateway_ip = ifconfig[2]

        print(f"[WIFI_COMM] Device IP: {device_ip}, Gateway IP: {gateway_ip}")

        target_ip = gateway_ip

        return create_persistent_connection(wifi_interface, target_ip, WIFI_COMM_PORT, max_retries)
    except Exception as e:
        print(f"ERROR - Failed to create WiFi connection: {e}")
        return None


def check_wifi_connection_status():
    """
    Check WiFi connection status and socket health.
    Updates global flags: communication, wifi_comm_enabled, wifi_socket.
    If WiFi disconnects, disable WiFi communication and close socket.
    If WiFi reconnects and socket is closed, attempt to reconnect.
    Uses cooldown to prevent redundant connection attempts.
    """
    global communication, wifi_comm_enabled, wifi_socket

    if wifi_nic is None:
        wifi_comm_enabled = False
        communication = False
        if wifi_socket is not None:
            try:
                wifi_socket.close()
            except Exception:
                pass
            wifi_socket = None
        return

    is_connected = wifi_nic.isconnected()

    if not is_connected:
        if wifi_comm_enabled or wifi_socket is not None:
            print("WARNING - WiFi disconnected, disabling WiFi communication")
            wifi_comm_enabled = False
            communication = False
            if wifi_socket is not None:
                try:
                    wifi_socket.close()
                except Exception:
                    pass
                wifi_socket = None
    else:
        return


def connect_hotspot_server(wifi_interface):
    global wifi_comm_enabled, communication, wifi_socket, wifi_nic, last_connection_attempt_time

    if wifi_interface is None or not wifi_interface.isconnected():
        print("WARNING - WiFi not connected, cannot initialize WiFi communication")
        wifi_comm_enabled = False
        communication = False
        wifi_socket = None
        wifi_nic = None
        return False

    try:
        last_connection_attempt_time = time.time()

        if wifi_socket is not None:
            try:
                wifi_socket.close()
            except Exception:
                pass
            wifi_socket = None
            wifi_comm_enabled = False
            communication = False

        new_socket = create_wifi_connection(wifi_interface, max_retries=3)

        if new_socket is not None:
            wifi_socket = new_socket
            wifi_nic = wifi_interface
            wifi_comm_enabled = True
            communication = True
            ifconfig = wifi_interface.ifconfig()
            target_ip = ifconfig[2]
            print(f"info - WiFi communication enabled, connected to {target_ip}:{WIFI_COMM_PORT}")
            update_hbstatus()
            return True
        else:
            print("WARNING - Failed to establish WiFi communication socket connection")
            wifi_comm_enabled = False
            communication = False
            wifi_socket = None
            wifi_nic = wifi_interface
            return False
    except Exception as e:
        print(f"ERROR - Failed to initialize WiFi communication: {e}")
        wifi_comm_enabled = False
        communication = False
        wifi_socket = None
        wifi_nic = wifi_interface
        return False


def get_wifi_comm_state():
    return {
        'wifi_comm_enabled': wifi_comm_enabled,
        'communication': communication,
        'socket': wifi_socket,
        'wifi_logging_enabled': wifi_logging_enabled
    }


def _extract_complete_messages(buffer):
    """
    Extract complete JSON messages from buffer.
    Messages are newline-delimited.
    Returns: (list of complete messages, remaining buffer)
    """
    messages = []
    lines = buffer.split('\n')

    # All lines except the last are complete (last might be incomplete)
    for line in lines[:-1]:
        line = line.strip()
        if line:  # Skip empty lines
            try:
                msg = ujson.loads(line)
                messages.append(msg)
            except Exception as e:
                print(f"[WIFI_READ] Failed to parse JSON line: {e}")
                print(f"[WIFI_READ] Problematic line: {line[:100]}...")

    # Return the last line as remaining buffer (might be incomplete)
    return messages, lines[-1]


async def wifi_socket_read_loop():
    print("info - Starting WiFi socket read loop")
    global wifi_socket, wifi_comm_enabled, communication, recv_timeout
    global _message_buffer

    consecutive_empty_reads = 0

    while True:
        try:
            if wifi_socket is None or not wifi_comm_enabled:
                await asyncio.sleep(3)
                consecutive_empty_reads = 0
                continue

            try:
                wifi_socket.settimeout(recv_timeout)
            except Exception as e:
                print(f"[WIFI_READ] Error setting socket timeout: {e}")
                await asyncio.sleep(3)
                continue

            try:
                data = wifi_socket.recv(1024)

                if data and len(data) > 0:
                    recv_timeout = 0.1
                    consecutive_empty_reads = 0

                    try:
                        data_str = data.decode('utf-8')
                        print(f"[WIFI_READ] Received {len(data_str)} bytes")

                        _message_buffer += data_str

                        while _message_buffer:
                            _message_buffer = _message_buffer.lstrip()

                            if not _message_buffer:
                                break

                            try:
                                message = ujson.loads(_message_buffer)
                                handle_message(message)
                                _message_buffer = ""
                                break

                            except ValueError:
                                newline_pos = _message_buffer.find('\n')
                                if newline_pos != -1:
                                    potential_message = _message_buffer[:newline_pos]
                                    try:
                                        message = ujson.loads(potential_message)
                                        print(
                                            "[WIFI_READ] Processing message: "
                                            f"{message.get('message_type', 'unknown')}"
                                        )
                                        handle_message(message)
                                        _message_buffer = _message_buffer[newline_pos + 1:]
                                        continue
                                    except Exception:
                                        pass
                                brace_count = 0
                                in_string = False
                                escape_next = False

                                for i, char in enumerate(_message_buffer):
                                    if escape_next:
                                        escape_next = False
                                        continue
                                    if char == '\\':
                                        escape_next = True
                                        continue
                                    if char == '"' and not escape_next:
                                        in_string = not in_string
                                    if not in_string:
                                        if char == '{':
                                            brace_count += 1
                                        elif char == '}':
                                            brace_count -= 1
                                            if brace_count == 0:
                                                potential_message = _message_buffer[:i + 1]
                                                try:
                                                    message = ujson.loads(potential_message)
                                                    print(
                                                        "[WIFI_READ] Processing message: "
                                                        f"{message.get('message_type', 'unknown')}"
                                                    )
                                                    handle_message(message)
                                                    _message_buffer = _message_buffer[i + 1:]
                                                    break
                                                except Exception:
                                                    pass
                                else:
                                    break

                    except Exception as e:
                        print(f"[WIFI_READ] Error processing data: {e}")
                        _message_buffer = ""

                    await asyncio.sleep(0.1)
                else:
                    recv_timeout = 0.1
                    consecutive_empty_reads += 1

                    if consecutive_empty_reads > 10:
                        await asyncio.sleep(5)
                    else:
                        await asyncio.sleep(0.05)

            except OSError as e:
                errno = getattr(e, 'errno', None)
                if errno == 116:
                    recv_timeout = 0.1
                    await asyncio.sleep(0.1)
                elif errno in (104, 107):
                    print(f"[WIFI_READ] Connection lost: {e}")
                    communication = False
                    wifi_comm_enabled = False
                    try:
                        wifi_socket.close()
                    except Exception:
                        pass
                    wifi_socket = None
                    _message_buffer = ""
                    recv_timeout = 0.1
                    consecutive_empty_reads = 0  # ← Added
                    await asyncio.sleep(3)
                else:
                    print(f"[WIFI_READ] Socket error: {e}")
                    recv_timeout = 0.1
                    await asyncio.sleep(0.5)  # ← Changed from 3 to 0.5

            except Exception as e:
                print(f"[WIFI_READ] Unexpected error: {e}")
                recv_timeout = 0.1
                await asyncio.sleep(0.5)  # ← Changed from 3 to 0.5

        except Exception as e:
            print(f"[WIFI_READ] Error in read loop: {e}")
            await asyncio.sleep(3)


def _get_file_save_root():
    """Return root path for saving received files (sdcard or flash)."""
    try:
        os.listdir("/sdcard")
        return "/sdcard"
    except OSError:
        return "/flash"


def _handle_start_file_transfer(message):
    global _file_transfer_state

    file_name = message.get("data")
    payload = message.get("payload", {})
    no_of_chunks = payload.get("no_of_chunks", 0)

    if not file_name:
        logger.error("[FILE_RECV] start_file_transfer missing file_name")
        return

    if _file_transfer_state and _file_transfer_state.get("file_handle"):
        try:
            _file_transfer_state["file_handle"].close()
        except Exception:
            pass

    root = _get_file_save_root()
    save_path = f"{root}/{file_name}"

    try:
        f = open(save_path, "wb")
        _file_transfer_state = {
            "file_name": file_name,
            "save_path": save_path,
            "file_handle": f,
            "expected_chunks": no_of_chunks,
            "received_chunks": 0,
            "chunks": {}  # Store chunks by index to handle out-of-order delivery
        }
        logger.info(f"[FILE_RECV] Started receiving file: {file_name} ({no_of_chunks} chunks expected)")
    except Exception as e:
        logger.error(f"[FILE_RECV] Failed to open file for write: {e}")
        _file_transfer_state = None


def _handle_file_chunk(message):
    if _file_transfer_state is None:
        logger.error("[FILE_RECV] file_chunk received without start_file_transfer")
        return

    data_b64 = message.get("data")
    chunk_index = message.get("payload")

    if data_b64 is None or chunk_index is None:
        logger.error("[FILE_RECV] file_chunk missing data or chunk_index")
        return

    try:
        import ubinascii
        chunk_bytes = ubinascii.a2b_base64(data_b64)

        _file_transfer_state["chunks"][chunk_index] = chunk_bytes
        _file_transfer_state["received_chunks"] += 1

        print(
            f"[FILE_RECV] Received chunk {chunk_index} ({len(chunk_bytes)} bytes) - "
            f"{_file_transfer_state['received_chunks']}/"
            f"{_file_transfer_state['expected_chunks']}"
        )

    except Exception as e:
        logger.error(f"[FILE_RECV] Failed to decode chunk {chunk_index}: {e}")


def _handle_end_file_transfer():
    global _file_transfer_state, recv_timeout

    if _file_transfer_state is None:
        logger.error("[FILE_RECV] end_file_transfer received without active transfer")
        return

    f = _file_transfer_state.get("file_handle")
    save_path = _file_transfer_state.get("save_path", "?")
    file_name = _file_transfer_state.get("file_name", "?")
    expected_chunks = _file_transfer_state.get("expected_chunks", 0)
    received_chunks = _file_transfer_state.get("received_chunks", 0)
    chunks = _file_transfer_state.get("chunks", {})

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
            logger.info(f"[FILE_RECV] Successfully saved file: {save_path} ({received_chunks} chunks)")
            send_data_to_app({
                "message_type": "end_file_transfer",
                "data:": {
                    "file_name": file_name,
                    "save_path": save_path,
                    "chunks_received": received_chunks,
                    "chunks_expected": expected_chunks,
                },
                "timestamp": time.time(),
            })
            import machine
            recv_timeout = 0.1
            _file_transfer_state = None
            logger.error("==== Rebooting the device in 10 seconds ==== ")
            time.sleep(10)
            try:
                os.sync()
            except (OSError, AttributeError):
                pass
            time.sleep(0.5)
            machine.reset()
        else:
            logger.error(f"[FILE_RECV] File incomplete: {received_chunks}/{expected_chunks} chunks received")
            send_data_to_app({
                "message_type": "end_file_transfer",
                "data:": {
                    "file_name": file_name,
                    "save_path": save_path,
                    "chunks_received": received_chunks,
                    "chunks_expected": expected_chunks,
                },
                "timestamp": time.time(),
            })
            recv_timeout = 0.1
            _file_transfer_state = None

    except Exception as e:
        logger.error(f"[FILE_RECV] Failed to close/save file: {e}")


def send_log_file(filename="main.log"):
    """
    Stream the main log file over the WiFi socket in chunks.
    This is triggered by the 'download_logs' command.
    Uses the same path as the logger so we read from where logs are actually written.
    """
    log_path = None
    try:
        FS_ROOT = None
        has_sdcard = True
        try:
            os.listdir("/sdcard")
        except OSError:
            has_sdcard = False
        if has_sdcard:
            FS_ROOT = "/sdcard"
        else:
            FS_ROOT = "/flash"
        log_path = f"{FS_ROOT}/logs/{filename}"

        if not log_path:
            logger.error("Log path not available, cannot send logs")
            return
        # 1) Send "start" message with metadata
        start_msg = {
            "message_type": "log_file_start",
            "file_name": "main.log",
            "timestamp": time.time(),
        }
        ok, _ = send_data_to_app(start_msg, 0.5)
        if not ok:
            print("[WIFI_COMM] Failed to send log_file_start")
            return

        # 2) Stream file contents in small chunks to avoid RAM pressure
        CHUNK_SIZE = 2048
        chunk_index = 0

        with open(log_path, "r") as f:
            global FILE_TRANSFER_BUFFER
            while True:
                FILE_TRANSFER_BUFFER = f.read(CHUNK_SIZE)
                if not FILE_TRANSFER_BUFFER:
                    break

                chunk_msg = {
                    "message_type": "log_file_chunk",
                    "file_name": "main.log",
                    "chunk_index": chunk_index,
                    "data": FILE_TRANSFER_BUFFER,
                }

                ok, _ = send_data_to_app(chunk_msg, 5)
                if not ok:
                    print(f"[WIFI_COMM] Failed to send log_file_chunk {chunk_index}")
                    return

                chunk_index += 1

        # 3) Send "end" message so receiver knows transfer is complete
        end_msg = {
            "message_type": "log_file_end",
            "file_name": "main.log",
            "total_chunks": chunk_index,
            "timestamp": time.time(),
        }
        send_data_to_app(end_msg, 0.5)
    except Exception as e:
        logger.error(f"[WIFI_COMM] Failed to stream log file '{log_path}': {e}")
        return


def update_hbstatus():
    data = apphandler.send_hb_data()
    parts = data.split(":")

    def _part(i, default=""):
        return parts[i].strip() if i < len(parts) else default
    # disarmed is 8th field (index 7); normalize to boolean for app
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
    # print(f"Printing heartbeat_dict: {heartbeat_dict}")
    msg = {
        "message_type": "heartbeat",
        "data": heartbeat_dict,
        "timestamp": time.time()
    }
    send_data_to_app(msg)


def report_radio_check_result(success_count, success_rate, transfer_rate):
    """Send radio connectivity check result to the app."""
    msg = {
        "message_type": "radio_check_result",
        "data": json.dumps({
            "success_count": success_count,
            "success_rate_percent": success_rate,
            "transfer_rate_msgs_per_sec": transfer_rate,
        }),
        "timestamp": time.time(),
    }
    send_data_to_app(msg)


def radio_check(target_addr=0, byte_count=0):
    async def _run_check():
        result = await apphandler.check_radio_connectivity_with(
            target_addr, num_messages=10, byte_count=byte_count
        )
        report_radio_check_result(result[0], result[1], result[2])

    try:
        loop = asyncio.get_event_loop()
        loop.create_task(_run_check())
    except Exception as e:
        logger.error(f"radio_check failed to schedule task: {e}")


def list_images():
    imagelist = apphandler.list_images()
    create_and_send_message("list_images", imagelist, 5)


def clear_image_queue():
    return apphandler.clear_image_queue()


def clear_all_queue():
    return apphandler.clear_all_queue()


def get_image(imagename):
    # TODO(akash) check
    imgfile = "/sdcard/" + imagename
    send_log_file(imgfile)


def send_image_in_chunks(image_path):
    try:
        filename = image_path.split('/')[-1]

        start_msg_data = {
            "file_name": filename,
            "file_path": image_path,
        }
        create_and_send_message("image_transfer_start", start_msg_data, timeout=1.0)

        chunk_index = 0
        with open(image_path, "rb") as f:
            while True:
                bytes_read = f.readinto(FILE_TRANSFER_BUFFER)

                if bytes_read == 0:
                    break

                chunk_data = bytes(FILE_TRANSFER_BUFFER[:bytes_read])

                import ubinascii
                chunk_data_b64 = ubinascii.b2a_base64(chunk_data).decode('utf-8').strip()

                chunk_msg_data = {
                    "file_name": filename,
                    "chunk_index": chunk_index,
                    "chunk_size": bytes_read,
                    "data": chunk_data_b64,
                }

                create_and_send_message("image_transfer_chunk", chunk_msg_data, timeout=5.0)
                logger.info(f"[IMAGE_TRANSFER] Sent chunk {chunk_index} ({bytes_read} bytes)")

                chunk_index += 1

                # time.sleep(0.1)

        end_msg_data = {
            "file_name": filename,
            "file_path": image_path,
            "total_chunks": chunk_index,
        }
        create_and_send_message("image_transfer_end", end_msg_data, timeout=1.0)

        return True

    except OSError as e:
        create_and_send_message("image_transfer_error",
                                f"File error: {str(e)}",
                                timeout=1.0)
        return False

    except MemoryError as e:
        create_and_send_message("image_transfer_error",
                                f"Memory error: {str(e)}",
                                timeout=1.0)
        return False

    except Exception as e:
        create_and_send_message("image_transfer_error",
                                f"Transfer failed: {str(e)}",
                                timeout=1.0)
        return False


def handle_message(message):
    global recv_timeout
    msg_type = message.get("message_type")
    if msg_type == "command":
        print(f"received command: {message}")
        handle_command(message)
    elif msg_type == "start_file_transfer":
        recv_timeout = 5.0
        _handle_start_file_transfer(message)
        create_and_send_message("ack", "start_file_transfer")
    elif msg_type == "file_chunk":
        recv_timeout = 5.0
        _handle_file_chunk(message)
    elif msg_type == "end_file_transfer":
        _handle_end_file_transfer()


def get_recent_logs():
    all_logs = apphandler.get_saved_logs()
    alength = len(all_logs)
    print(alength)
    start = alength - 10
    start = 0 if start < 0 else start
    for i in range(start, alength):
        create_and_send_message("log", all_logs[i], timeout=2.0)


def handle_command(message):
    global wifi_socket, recv_timeout
    command = message.get("data")
    if (command == "ping"):  # Ping Pong
        send_data_to_app("pong")
    elif (command == "reboot"):  # Reboot the device
        import machine
        logger.error("==== Rebooted by APP --- will reboot in 10 seconds ==== ")
        _close_socket_safely(wifi_socket)
        wifi_socket = None
        create_and_send_message("disconnect", "reboot")
        time.sleep(10)
        machine.reset()
    elif (command == "set_disarmed"):  # Set disarmed status
        payload = message.get("payload")
        if payload and payload == "arm":
            apphandler.arm()
            update_hbstatus()
        elif payload and payload == "disarm":
            apphandler.disarm()
            update_hbstatus()
        else:
            logger.error("invalid value provided")
    elif (command == "download_logs"):
        get_recent_logs()

    elif (command == "show_status"):  # Show status of the device
        update_hbstatus()
        recv_timeout = 2.0
    elif (command == "radio_check"):  # Radio check
        payload = message.get("payload")
        radio_check(int(payload.get("target_addr", 0)), int(payload.get("byte_count", 0)))

    elif (command == "list_images"):
        list_images()
    elif (command == "clear_image_queue"):
        clear_image_queue()
    elif (command == "download_image"):
        image_path = message.get("payload")
        if image_path:
            send_image_in_chunks(image_path)
        else:
            logger.error("no image path provided")
    elif (command == "clear_all_queue"):
        clear_all_queue()
    else:
        logger.info(f"Unknown command: {command}")


def create_and_send_message(message_type, data, timeout=0.5):
    msg = {
        "message_type": message_type,
        "data": data,
        "timestamp": time.time()
    }
    send_data_to_app(msg, timeout)

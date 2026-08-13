"""
GPS Driver for TracX-1b Module
Handles GPS/GNSS functionality only
Based on EC200U AT commands
"""

import time
from machine import UART
import config
import logger
from clock_utils import get_epoch_ms

# Configuration
UART_ID = 1
BAUDRATE = 115200


class GPSDriver:
    """GPS driver for TracX-1b module (GPS/GNSS only)"""
    
    def __init__(self, uart=None, uart_id=UART_ID, baudrate=BAUDRATE):
        """
        Initialize GPS driver with UART configuration
        Args:
            uart: Optional shared UART instance (if None, will create new one)
            uart_id: UART ID if creating new UART (default: 1)
            baudrate: Baudrate if creating new UART (default: 115200)
        """
        self.uart = uart  # Use shared UART if provided
        self.uart_id = uart_id
        self.baudrate = baudrate
        self.gps_initialized = False
        saved_lat, saved_lon, saved_gps_time = config.get_gps_location()
        if saved_lat and saved_lon:
            self.lat = saved_lat
            self.lon = saved_lon
            self.gps_time = saved_gps_time
            logger.info(f"[GPS] ✔✔✔ GPS location loaded from store: {self.lat}, {self.lon}, {self.gps_time}")
        else:
            self.lat = 0
            self.lon = 0
            self.gps_time = 0
            logger.info(f"[GPS] ✘✘✘ GPS location not found in store")
    
    def _send_at(self, cmd, wait_ms=1000, retry=3):
        """Send AT command and return response"""
        if self.uart is None:
            self.uart = UART(self.uart_id, self.baudrate, timeout=2000)
            logger.debug("[GPS] UART initialized, waiting for module...")
            time.sleep_ms(2000)  # Wait for module to initialize
        # If shared UART is provided, it's already initialized
        
        # Clear buffer before sending
        while self.uart.any():
            self.uart.read()
        
        for attempt in range(retry):
            # logger.debug(f"[GPS] Sending: {cmd} (attempt {attempt+1}/{retry})")
            self.uart.write((cmd + "\r\n").encode())
            
            end = time.ticks_ms() + wait_ms
            resp = b""
            while time.ticks_diff(end, time.ticks_ms()) > 0:
                if self.uart.any():
                    resp += self.uart.read()
                time.sleep_ms(10)
            
            # Check if we got a response
            if len(resp) > 0:
                try:
                    resp_preview = resp[:200] if len(resp) > 200 else resp
                    resp_text = resp_preview.decode('ascii')
                    # logger.debug(f"[GPS] Response: {resp_text}")
                except:
                    logger.info(f"[GPS] Response: {len(resp)} bytes (decode failed)")
                return resp
            elif attempt < retry - 1:
                # No response, retry after delay
                logger.debug(f"[GPS] No response, retrying...")
                time.sleep_ms(500)
        
        # No response after retries
        logger.debug("[GPS] Response: (empty after retries)")
        return b""
    
    def _check_response(self, resp, expected="OK"):
        """Check if response contains expected string"""
        return expected.encode() in resp
    
    def initialize_gps(self):
        """Enable GPS/GNSS on TracX-1b module"""
        logger.info("[GPS] Initializing GPS...")
        
        # Initialize UART if not already done (and not shared)
        if self.uart is None:
            self.uart = UART(self.uart_id, self.baudrate, timeout=2000)
            logger.info("[GPS] UART initialized, waiting for module...")
            time.sleep_ms(2000)  # Wait for module to initialize
        # If shared UART is provided, it's already initialized
        
        # Test AT command
        resp = self._send_at("AT", 1000, retry=3)
        if not self._check_response(resp):
            logger.error("[GPS] No AT response!")
            return False
        # logger.debug("[GPS] Module responding to AT")
        
        # Disable echo
        self._send_at("ATE0", 500)

        # Exit module sleep before enabling GNSS (paired with enter_gps_sleep)
        self._send_at("AT+QSCLK=0", 2000)

        # Disable GPS first (clean state for script rerun without replug)
        self._send_at("AT+QGPS=0", 2000)
        time.sleep_ms(500)

        # Enable GPS
        resp = self._send_at("AT+QGPS=1", 5000)
        if self._check_response(resp):
            self.gps_initialized = True
            logger.info("[GPS] GPS initialized successfully")
            return True
        
        logger.error("[GPS] GPS initialization failed")
        return False

    def enter_gps_sleep(self, sleep_module=True):
        """Turn off GNSS; optionally enable module sleep (QSCLK). Skip QSCLK when cellular CC needs the modem awake."""
        logger.info("[GPS] Entering GNSS sleep...")
        self._send_at("AT+QGPSEND", 5000)
        if sleep_module:
            self._send_at("AT+QSCLK=1", 2000)
        self.gps_initialized = False
        logger.info("[GPS] GNSS sleep enabled")
    
    def get_gps_location(self):
        """
        Query GPS location
        Returns: (lat, lon, time_str) tuple or (None, None, None) if no fix
        """
        if not self.gps_initialized:
            if not self.initialize_gps():
                return None, None, None
        
        resp = self._send_at("AT+QGPSLOC?", 3000)
        lat, lon, time_str = self._parse_gps_response(resp)
        if lat is not None and lon is not None:
            self.lat = lat
            self.lon = lon
            self.gps_time = get_epoch_ms()
            config.set_gps_location(lat, lon, self.gps_time)
            logger.info(f"[GPS] ✔✔✔ GPS location saved to store: {lat}, {lon}, {self.gps_time}")
        return lat, lon, time_str

    def get_saved_gps_location(self):
        """
        Return last cached fix from get_gps_location (no UART).
        Returns: (lat, lon) or (None, None) if never fixed.
        """
        return self.lat, self.lon
    
    def _utc_to_local(self, dd, mo, yy, hh, mm, ss, tz_offset=5.5):
        """Convert UTC time to local time using timezone offset (IST = UTC+5:30)"""
        dd, mo = int(dd), int(mo)
        # Add timezone offset to UTC time (in seconds)
        secs = int(hh) * 3600 + int(mm) * 60 + int(ss) + int(tz_offset * 3600)
        # Handle day rollover (next/previous day)
        if secs >= 86400:
            secs -= 86400
            dd += 1
        elif secs < 0:
            secs += 86400
            dd -= 1
        # Convert back to hours, minutes, seconds
        h = secs // 3600
        m = (secs % 3600) // 60
        s = secs % 60
        return "%02d/%02d/20%s %02d:%02d:%02d" % (dd, mo, yy, h, m, s)
    
    def _parse_gps_response(self, resp):
        """Parse AT+QGPSLOC? response - returns (lat, lon, time_str) or (None, None, None)"""
        try:
            text = resp.decode("ascii")
        except:
            return None, None, None
        
        if "+CME ERROR" in text:
            # 516 = GPS not fixed (cold start, weak signal, indoors); 505 = no fix in newer firmware
            if "516" in text or "505" in text:
                logger.debug("[GPS] No fix yet (CME 516/505) - wait for satellite lock, try outdoors")
            return None, None, None
        if "+QGPSLOC:" not in text:
            return None, None, None
        
        for line in text.split("\n"):
            if not line.startswith("+QGPSLOC:"):
                continue
            try:
                # Format: +QGPSLOC: hhmmss.sss,ddmm.mmmmN/S,dddmm.mmmmE/W,...,ddmmyy
                parts = line.split(":")[1].strip().split(",")
                if len(parts) < 10:
                    continue

                utc, lat_f, lon_f, date = parts[0], parts[1], parts[2], parts[9]
                hh, mm, ss = utc[0:2], utc[2:4], utc[4:6]
                dd, mo, yy = date[0:2], date[2:4], date[4:6]

                # Convert UTC to local time (IST = UTC+5:30)
                timestr = self._utc_to_local(dd, mo, yy, hh, mm, ss, tz_offset=5.5)

                # Convert NMEA format to decimal degrees
                def to_deg(s, is_lat):
                    d = int(s[0:2 if is_lat else 3])
                    m = float(s[2 if is_lat else 3:])
                    return d + m / 60.0

                lat = to_deg(lat_f[:-1], True)
                lon = to_deg(lon_f[:-1], False)

                # Handle hemisphere (S/W = negative)
                if lat_f[-1] == "S":
                    lat = -lat
                if lon_f[-1] == "W":
                    lon = -lon

                return lat, lon, timestr
            except (ValueError, IndexError, TypeError) as e:
                logger.warning(f"[GPS] Parse error for line: {e}")
                continue
   # logger.debug("[GPS] Module responding to AT")
        return None, None, None
    
    def get_gps_time_components(self, time_str):
        """
        Parse GPS time string and return RTC-compatible tuple
        Args: time_str in format "DD/MM/YYYY HH:MM:SS" (local time)
        Returns: (year, month, day, hour, minute, second, weekday, yearday) tuple or None
        """
        if time_str is None:
            return None
        
        try:
            # Parse time string: "DD/MM/YYYY HH:MM:SS"
            date_part, time_part = time_str.split(" ")
            dd, mm, yyyy = date_part.split("/")
            hh, mm_sec, ss = time_part.split(":")
            
            # RTC.datetime format: (year, month, day, weekday, hour, minute, second, microsecond)
            # weekday: 0=Monday, 6=Sunday (can be 0 for now)
            # yearday: day of year (can be 0 for now)
            return (int(yyyy), int(mm), int(dd), 0, int(hh), int(mm_sec), int(ss), 0)
        except Exception as e:
            logger.error(f"[GPS] Failed to parse time string '{time_str}': {e}")
            return None

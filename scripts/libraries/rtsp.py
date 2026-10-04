# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2026 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2026 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# rtsp - asyncio RTSP server streaming MJPEG or H.264 video, with optional
# L16 PCM audio, over TCP-interleaved or UDP RTP.
#
#   server = rtsp.rtsp_server(network_if)
#   asyncio.run(server.stream(lambda p, s: csi0.snapshot()))          # MJPEG
#   asyncio.run(server.stream_h264(enc, lambda p, s: csi0.snapshot()))
#
# Media is packetized into one preallocated buffer (no per-packet
# allocations); a slow or absent client drops packets rather than stalling
# the camera loop.

import random
import re
import socket
import struct
import time

try:
    import asyncio
except ImportError:
    import uasyncio as asyncio

try:
    import image
except ImportError:
    image = None  # host testing: H.264/audio paths don't need it

_MTU = 1400
_RTP_OVERHEAD = 16  # $-framing + RTP header


def _sleep_ms(ms):
    return asyncio.sleep(ms / 1000)


def _ticks_90khz():
    try:
        return time.ticks_us() * 9 // 100
    except AttributeError:
        return int(time.monotonic() * 90000)


class rtsp_server:
    def __init__(self, network_if, port=554):  # private
        self._network = network_if
        self._myip = self._network.ifconfig()[0]
        self._myaddr = (self._myip, port)
        self._setup_cb = None
        self._play_cb = None
        self._pause_cb = None
        self._teardown_cb = None
        self._pathname = ""
        self._session = random.getrandbits(30)
        self._transport_is_tcp = False
        self._playing = False
        self._playing_session = 0
        self._sequence_number = random.getrandbits(16)
        self._ssrc = random.getrandbits(30)
        self._writer = None
        self._lock = asyncio.Lock()
        self._needs_idr = True
        self._last_ts = 0
        # Per track (0=video, 1=audio): interleaved channel or UDP address.
        self._channels = [0, 2]
        self._udp_addr = [None, None]
        self._udp_sock = None
        self._track_setup = [False, False]
        # Preallocated packet buffer: [$ C len][RTP header][payload].
        self._pkt = bytearray(_MTU + _RTP_OVERHEAD)
        self._pkt_mv = memoryview(self._pkt)
        self._audio_rate = 0  # set by the rtsp_h264 subclass
        self._audio_channels = 1
        print("IP Address:Port %s:%d\nRunning..." % self._myaddr)

    def register_setup_cb(self, cb):  # public
        self._setup_cb = cb

    def register_play_cb(self, cb):  # public
        self._play_cb = cb

    def register_pause_cb(self, cb):  # public
        self._pause_cb = cb

    def register_teardown_cb(self, cb):  # public
        self._teardown_cb = cb

    # ------------------------------------------------------------------ #
    # Socket plumbing
    # ------------------------------------------------------------------ #
    async def _send_all(self, data):  # private
        # Control responses and interleaved RTP share the TCP stream; the
        # lock keeps packets whole across the video and audio tasks.
        async with self._lock:
            w = self._writer
            if w is None:
                return  # client disconnected - drop
            w.write(data)
            await w.drain()

    def _send_udp(self, mv, track):  # private
        try:
            self._udp_sock.sendto(mv, self._udp_addr[track])
        except OSError:
            pass  # drop, never stall the camera loop

    async def _send_packet(self, track, payload_len):  # private
        # Packet was assembled in self._pkt at offset 4 (RTP header at 4).
        if self._transport_is_tcp:
            struct.pack_into(">BBH", self._pkt, 0, 0x24,
                             self._channels[track], 12 + payload_len)
            await self._send_all(self._pkt_mv[:16 + payload_len])
        else:
            self._send_udp(self._pkt_mv[4:16 + payload_len], track)

    def _timestamp(self):  # private
        # 90 kHz media clock; strictly monotonic so back-to-back frames
        # (e.g. sent in a burst after a stall) never share a timestamp.
        ts = _ticks_90khz()
        if ts <= self._last_ts:
            ts = self._last_ts + 1
        self._last_ts = ts
        return ts

    def _rtp_header(self, pt_marker, seq, timestamp, track=0):  # private
        struct.pack_into(">BBHII", self._pkt, 4, 0x80, pt_marker, seq,
                         timestamp & 0xFFFFFFFF, self._ssrc + track)

    # ------------------------------------------------------------------ #
    # RTSP control protocol (structure follows the original blocking
    # implementation; responses are strings since they are rare)
    # ------------------------------------------------------------------ #
    async def _send_rtsp_response(self, code, name, extra=""):  # private
        await self._send_all(memoryview(
            ("RTSP/1.0 %d %s\r\n%s\r\n" % (code, name, extra)).encode()))

    async def _send_rtsp_response_cseq(self, code, name, seq, extra=""):  # private
        await self._send_rtsp_response(code, name, "CSeq: %d\r\n%s" % (seq, extra))

    async def _send_rtsp_response_ok(self, seq, extra=""):  # private
        await self._send_rtsp_response_cseq(200, "OK", seq, extra)

    def _sdp_media(self):  # overridden by rtsp_h264
        sdp = "m=video 0 RTP/AVP 26\r\na=control:trackID=0\r\n"
        if self._audio_rate:
            sdp += ("m=audio 0 RTP/AVP 97\r\n"
                    "a=rtpmap:97 L16/%d/%d\r\n"
                    "a=control:trackID=1\r\n"
                    % (self._audio_rate, self._audio_channels))
        return sdp

    def _sdp(self):  # private
        return ("v=0\r\no=- %d %d IN IP4 %s\r\ns=OpenMV Video\r\nt=0 0\r\n" % (
            random.getrandbits(30), random.getrandbits(30), self._myip)) + self._sdp_media()

    async def _parse_rtsp_request(self, data):  # private
        s = list(
            filter(lambda x: x and not x.startswith("User-Agent"), data.decode().splitlines())
        )
        if s and len(s) >= 2:
            line0 = s[0].split(" ")
            request = line0[0]
            self._pathname = re.sub("rtsp(u)?://[a-zA-Z0-9\\-\\.]+(:\\d+)?(/)?", "/", line0[1])
            track = 1 if self._pathname.endswith("trackID=1") else 0
            if self._pathname != "/" and self._pathname.endswith("/"):
                self._pathname = self._pathname[:-1]
            if line0[2] == "RTSP/1.0":
                if len(s) >= 3 and s[1].split(" ")[0] != "CSeq:":
                    for i in range(2, len(s)):
                        if s[i].split(" ")[0] == "CSeq:":
                            temp = s[i]
                            s[i] = s[1]
                            s[1] = temp
                            break
                line1 = s[1].split(" ")
                if line1[0] == "CSeq:":
                    seq = int(line1[1])
                    if request == "OPTIONS":
                        await self._send_rtsp_response_ok(
                            seq, "Public: DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN\r\n"
                        )
                        return
                    elif request == "DESCRIBE":
                        payload = self._sdp()
                        await self._send_rtsp_response_ok(
                            seq,
                            "Content-Type: application/sdp\r\nContent-Length: %d\r\n\r\n%s"
                            % (len(payload) + 2, payload),
                        )
                        return
                    elif request == "SETUP":
                        for line in s[2:]:
                            m = re.match(
                                "Transport: RTP/AVP(/UDP)?;unicast;client_port=(\\d+)-(\\d+)", line
                            )
                            if m:
                                self._transport_is_tcp = False
                                self._udp_addr[track] = (
                                    self._client_addr[0],
                                    int(m.group(2)),
                                )
                                self._track_setup[track] = True
                                if self._udp_sock is None:
                                    self._udp_sock = socket.socket(
                                        socket.AF_INET, socket.SOCK_DGRAM)
                                    self._udp_sock.setblocking(False)
                                await self._send_rtsp_response_ok(
                                    seq,
                                    "%s;server_port=%s-%s;ssrc=%d\r\nSession: %d\r\n"
                                    % (line, m.group(2), m.group(3),
                                       self._ssrc, self._session),
                                )
                                if self._setup_cb:
                                    self._setup_cb(self._pathname, self._session)
                                return
                            m = re.match(
                                "Transport: RTP/AVP/TCP;unicast;interleaved=(\\d+)-(\\d+)", line
                            )
                            if m:
                                self._transport_is_tcp = True
                                self._channels[track] = int(m.group(1))
                                self._track_setup[track] = True
                                await self._send_rtsp_response_ok(
                                    seq, "%s\r\nSession: %d\r\n" % (line, self._session)
                                )
                                if self._setup_cb:
                                    self._setup_cb(self._pathname, self._session)
                                return
                    elif request == "PLAY":
                        self._playing = True
                        self._playing_session = self._session
                        self._needs_idr = True
                        await self._send_rtsp_response_ok(
                            seq, "Session: %d\r\n" % self._session
                        )
                        if self._play_cb:
                            self._play_cb(self._pathname, self._playing_session)
                        return
                    elif request == "PAUSE":
                        self._playing = False
                        await self._send_rtsp_response_ok(seq)
                        if self._pause_cb:
                            self._pause_cb(self._pathname, self._session)
                        return
                    elif request == "TEARDOWN":
                        self._playing = False
                        await self._send_rtsp_response_ok(seq)
                        if self._teardown_cb:
                            self._teardown_cb(self._pathname, self._session)
                        return
                    else:
                        await self._send_rtsp_response_cseq(501, "Not Implemented", seq)
                        return
        await self._send_rtsp_response(400, "Bad Request")

    async def _handle_client(self, reader, writer):  # private
        if self._writer is not None:
            writer.close()
            return
        self._writer = writer
        self._client_addr = writer.get_extra_info("peername") or ("0.0.0.0", 0)
        self._session = random.getrandbits(30)
        self._ssrc = random.getrandbits(30)
        try:
            while True:
                first = await reader.readexactly(1)
                if not first:
                    break
                if first[0] == 0x24:  # interleaved RTP/RTCP frame - discard
                    hdr = await reader.readexactly(3)
                    await reader.readexactly((hdr[1] << 8) | hdr[2])
                    continue
                # Accumulate one text request up to the blank line.
                req = bytearray(first)
                req += await reader.readline()
                while True:
                    line = await reader.readline()
                    if not line:
                        break
                    req += line
                    if line == b"\r\n":
                        break
                await self._parse_rtsp_request(req)
        except (OSError, EOFError):
            pass
        finally:
            if self._playing:
                self._playing = False
                if self._teardown_cb:
                    self._teardown_cb(self._pathname, self._playing_session)
            self._track_setup[0] = False
            self._track_setup[1] = False
            self._writer = None
            try:
                writer.close()
            except OSError:
                pass

    # ------------------------------------------------------------------ #
    # RTP payloaders (packets assembled in the preallocated buffer)
    # ------------------------------------------------------------------ #
    async def _send_rtp_jpeg(self, img, quality, timestamp):  # private
        w8 = img.width() // 8
        h8 = img.height() // 8
        if w8 >= 256 or h8 >= 256:
            raise ValueError("Maximum size is 2040x2040")
        mv = memoryview(img)
        size = img.size()
        max_payload = _MTU - 8
        i = 0
        pkt = self._pkt
        while i < size:
            n = min(size - i, max_payload)
            last = (i + n) >= size
            self._rtp_header(0x9A if last else 0x1A,
                             self._sequence_number, timestamp)
            self._sequence_number = (self._sequence_number + 1) & 0xFFFF
            struct.pack_into(">IBBBB", pkt, 16, i, 0, quality, w8, h8)
            pkt[24:24 + n] = mv[i:i + n]
            await self._send_packet(0, 8 + n)
            i += n

    # ------------------------------------------------------------------ #
    # Media loops
    # ------------------------------------------------------------------ #
    def _client_ready(self, track):  # private
        return (self._playing and self._track_setup[track]
                and (self._writer is not None or self._udp_addr[track] is not None))

    async def _serve(self, media_loop, audio_loop):  # private
        server = await asyncio.start_server(
            self._handle_client, "0.0.0.0", self._myaddr[1])
        tasks = [asyncio.create_task(media_loop())]
        if audio_loop:
            tasks.append(asyncio.create_task(audio_loop()))
        await asyncio.gather(*tasks)

    async def stream(self, image_callback, quality=90, _audio_loop=None):  # public
        """Stream MJPEG video forever."""

        async def media_loop():
            while True:
                if self._client_ready(0):
                    img = image_callback(self._pathname, self._session)
                    img = img.to_jpeg(quality=quality,
                                      subsampling=image.JPEG_SUBSAMPLING_422)
                    try:
                        await self._send_rtp_jpeg(img, quality, self._timestamp())
                    except OSError:
                        pass
                    await _sleep_ms(0)
                else:
                    await _sleep_ms(100)

        await self._serve(media_loop, _audio_loop)

# This file is part of the OpenMV project.
#
# Copyright (c) 2013-2026 Ibrahim Abdelkader <iabdalkader@openmv.io>
# Copyright (c) 2013-2026 Kwabena W. Agyeman <kwagyeman@openmv.io>
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# rtsp_h264 - extends the rtsp server with H.264 video (RFC 6184) and L16
# PCM audio streaming. Split from rtsp.py so boards without the hardware
# codec do not carry this code in their firmware.
#
#   server = rtsp_h264.rtsp_server(network_if)
#   asyncio.run(server.stream_h264(enc, lambda p, s: csi0.snapshot()))

import random
from array import array
from mp4 import nal_bounds, nal_units
import rtsp
from rtsp import _sleep_ms, _MTU

try:
    from ulab import numpy as np
except ImportError:
    import numpy as np  # host testing


class rtsp_server(rtsp.rtsp_server):
    def __init__(self, network_if, port=554):  # private
        super().__init__(network_if, port)
        self._audio_sequence_number = random.getrandbits(16)
        self._audio_samples = 0
        self._scratch = None  # H.264 Annex-B scratch, sized on first use
        self._scratch_mv = None
        self._bounds = array("I", bytes(64 * 4))
        self._sps = None
        self._pps = None
        self._h264 = False
        # int16 alias of the packet buffer for vectorized L16 byteswap.
        self._pkt_i16 = np.frombuffer(self._pkt, dtype=np.int16)

    def _sdp_media(self):  # private
        if not self._h264:
            sdp = "m=video 0 RTP/AVP 26\r\na=control:trackID=0\r\n"
        else:
            import binascii
            plid = binascii.hexlify(self._sps[1:4]).decode()
            sps = binascii.b2a_base64(self._sps)[:-1].decode()
            pps = binascii.b2a_base64(self._pps)[:-1].decode()
            sdp = ("m=video 0 RTP/AVP 96\r\n"
                   "a=rtpmap:96 H264/90000\r\n"
                   "a=fmtp:96 packetization-mode=1; profile-level-id=%s; "
                   "sprop-parameter-sets=%s,%s\r\n"
                   "a=control:trackID=0\r\n" % (plid, sps, pps))
        if self._audio_rate:
            sdp += ("m=audio 0 RTP/AVP 97\r\n"
                    "a=rtpmap:97 L16/%d/%d\r\n"
                    "a=control:trackID=1\r\n"
                    % (self._audio_rate, self._audio_channels))
        return sdp

    # ------------------------------------------------------------------ #
    # RTP payloaders (packets assembled in the preallocated buffer)
    # ------------------------------------------------------------------ #
    async def _send_nal(self, src, begin, end, timestamp, last):  # private
        pkt = self._pkt
        n = end - begin
        if n <= _MTU:
            self._rtp_header(0xE0 if last else 0x60,  # PT 96 (+marker)
                             self._sequence_number, timestamp)
            self._sequence_number = (self._sequence_number + 1) & 0xFFFF
            pkt[16:16 + n] = src[begin:end]
            await self._send_packet(0, n)
            return
        # FU-A fragmentation (RFC 6184): 2-byte FU header per fragment.
        nal0 = src[begin]
        begin += 1
        first = 0x80
        while begin < end:
            n = min(end - begin, _MTU - 2)
            final = (begin + n) >= end
            self._rtp_header(0xE0 if (last and final) else 0x60,
                             self._sequence_number, timestamp)
            self._sequence_number = (self._sequence_number + 1) & 0xFFFF
            pkt[16] = (nal0 & 0xE0) | 28  # FU indicator
            pkt[17] = first | (0x40 if final else 0) | (nal0 & 0x1F)
            pkt[18:18 + n] = src[begin:begin + n]
            await self._send_packet(0, 2 + n)
            begin += n
            first = 0

    async def _send_rtp_h264(self, au, keyframe, timestamp):  # private
        if isinstance(au, memoryview):
            n = len(au)
            if self._scratch is None or len(self._scratch) < n:
                self._scratch = bytearray(max(n, 65536))
                self._scratch_mv = memoryview(self._scratch)
            self._scratch[:n] = au
            src = self._scratch
            smv = self._scratch_mv
        else:
            n = len(au)
            src = au
            smv = memoryview(au)
        nals = nal_bounds(src, self._bounds, n)
        if keyframe:
            # Repeat SPS/PPS in-band so late joiners can sync, unless the
            # access unit already carries its own (a repeat would register
            # as a second access unit with the same timestamp downstream).
            k = 0
            while k < nals and src[self._bounds[k * 2]] & 0x1F == 9:
                k += 1
            if k >= nals or src[self._bounds[k * 2]] & 0x1F != 7:
                await self._send_nal(self._sps, 0, len(self._sps), timestamp, False)
                await self._send_nal(self._pps, 0, len(self._pps), timestamp, False)
        k = 0
        while k < nals:
            begin = self._bounds[k * 2]
            end = self._bounds[k * 2 + 1]
            k += 1
            if src[begin] & 0x1F == 9:  # drop access unit delimiters
                continue
            await self._send_nal(smv, begin, end, timestamp, k == nals)

    async def _send_rtp_l16(self, pcm, timestamp):  # private
        # L16 is big-endian; the mic produces little-endian - copy into
        # the packet buffer as int16 and byteswap in place (vectorized,
        # no allocations beyond small views).
        src = np.frombuffer(pcm, dtype=np.int16)
        n = len(src)
        i = 0
        while i < n:
            m = min(n - i, (_MTU & ~1) >> 1)  # samples per packet
            self._rtp_header(0x61, self._audio_sequence_number,  # PT 97
                             timestamp + i // self._audio_channels, 1)
            self._audio_sequence_number = (self._audio_sequence_number + 1) & 0xFFFF
            seg = self._pkt_i16[8:8 + m]  # payload starts at byte 16
            seg[:] = src[i:i + m]
            seg.byteswap(inplace=True)
            await self._send_packet(1, m * 2)
            i += m

    # ------------------------------------------------------------------ #
    # Media loops
    # ------------------------------------------------------------------ #
    def _make_audio_loop(self, audio_callback,
                         audio_rate, audio_channels):  # private
        self._audio_rate = audio_rate
        self._audio_channels = audio_channels
        if not audio_rate:
            return None
        if audio_callback is None:
            # The caller owns the audio driver: create an mp4.MicSource,
            # pass mic.callback to audio.start_streaming(), and hand
            # mic.read in here.
            raise ValueError("audio_callback required with audio_rate")

        async def audio_loop():
            while True:
                pcm = audio_callback()
                if pcm and self._client_ready(1):
                    try:
                        await self._send_rtp_l16(pcm, self._audio_samples)
                    except OSError:
                        pass
                    self._audio_samples += (len(pcm) >> 1) // self._audio_channels
                else:
                    await _sleep_ms(5)

        return audio_loop

    async def stream(self, image_callback, quality=90, audio_rate=0,
                     audio_channels=1, audio_callback=None):  # public
        """Stream MJPEG video forever, with L16 PCM audio when audio_rate
        is set (audio_callback returns pending PCM, e.g. MicSource.read)."""
        self._h264 = False
        await super().stream(image_callback, quality,
                             _audio_loop=self._make_audio_loop(
                                 audio_callback, audio_rate, audio_channels))

    async def stream_h264(self, encoder, image_callback, audio_rate=0,
                          audio_channels=1, audio_callback=None):  # public
        """Stream H.264 from a codec.H264Encoder forever, with L16 PCM
        audio when audio_rate is set (audio_callback returns pending PCM,
        e.g. MicSource.read). The encoder's SPS/PPS go out in the SDP and
        are repeated in-band at every keyframe for late joiners."""
        self._h264 = True
        # Split the encoder's Annex-B config blob into SPS and PPS.
        for nal in nal_units(encoder.sps_pps()):
            if nal[0] & 0x1F == 7:
                self._sps = bytes(nal)
            elif nal[0] & 0x1F == 8:
                self._pps = bytes(nal)

        async def media_loop():
            while True:
                if self._client_ready(0):
                    img = image_callback(self._pathname, self._session)
                    au = encoder.encode(img, keyframe=self._needs_idr)
                    self._needs_idr = False
                    keyframe = encoder.keyframe()
                    try:
                        await self._send_rtp_h264(au, keyframe, self._timestamp())
                    except OSError:
                        pass
                    await _sleep_ms(0)
                else:
                    await _sleep_ms(100)

        await self._serve(media_loop, self._make_audio_loop(
            audio_callback, audio_rate, audio_channels))

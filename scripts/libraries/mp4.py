# This file is part of the OpenMV project.
#
# Copyright (c) 2026 OpenMV, LLC.
#
# This work is licensed under the MIT license, see the file LICENSE for details.
#
# mp4 - Fragmented MP4 (fMP4) writer for H.264 video with optional PCM audio.
#
# Writes an ISO-BMFF fragmented MP4: one init segment (ftyp+moov) followed by
# a moof+mdat pair per fragment. Fragments are cut at keyframes (or at
# fragment_frames, whichever comes first), so a power loss costs at most the
# open fragment, and each fragment goes out as a few large back-to-back
# sequential writes (efficient on SD cards, especially while WiFi is active).
#
# Video input is one H.264 access unit per write() call, in Annex-B format -
# exactly what a hardware encoder or a raw .h264 stream produces. Audio input
# is raw signed 16-bit little-endian PCM via write_audio(). Note that PCM
# audio tracks in MP4 play in VLC/ffmpeg/QuickTime but not in the built-in
# Windows players or browsers - remux on the desktop for those:
# ffmpeg -i in.mp4 -c:v copy -c:a aac out.mp4
#
# The steady-state write path is allocation-free by design: all buffers are
# preallocated at construction (sized by buffer_size/fragment_frames/
# audio_buffer), sample metadata lives in fixed array('I') slots, headers are
# built with struct.pack_into, and Annex-B parsing uses C-speed find() on a
# scratch buffer instead of per-byte Python loops. Only tiny transient
# memoryview slice objects are created per fragment.
#
# Reusable pieces:
#   - Mp4 accepts a path OR any object with a write() method (it never
#     seeks), so the same class records to SD, a socket, or a buffer.
#   - nal_bounds()/nal_units() parse the NALs of an Annex-B access unit;
#     other H.264 consumers (e.g. an RTP packetizer) should reuse them
#     instead of re-implementing start-code parsing.

import struct
from array import array

_VIDEO_TIMESCALE = 90000  # MPEG convention for H.264 track timescales.

# ISO-BMFF trun sample flags.
_FLAG_KEY = 0x02000000  # sample_depends_on = no others (sync sample)
_FLAG_NONKEY = 0x01010000  # depends on others + is_non_sync_sample

_START = b"\x00\x00\x01"


def nal_bounds(data, bounds, end=-1):
    """Fill the preallocated array('I') `bounds` with (start, end) offset
    pairs for each NAL unit (start codes stripped) in the first `end` bytes
    of the Annex-B buffer `data`, and return the NAL count. Allocation-free;
    `data` must support find() (bytes/bytearray - copy a memoryview into a
    scratch bytearray first)."""
    n = len(data) if end < 0 else end
    limit = len(bounds) >> 1
    count = 0
    i = data.find(_START, 0, n)
    if i < 0:
        raise ValueError("no Annex-B start code in access unit")
    while i >= 0:
        begin = i + 3
        i = data.find(_START, begin, n)
        # A 4-byte start code puts one zero before the 3-byte form.
        stop = n if i < 0 else (i - 1 if data[i - 1] == 0 else i)
        if count >= limit:
            raise ValueError("too many NAL units in access unit")
        bounds[count * 2] = begin
        bounds[count * 2 + 1] = stop
        count += 1
    return count


def nal_units(au):
    """Yield the NAL units of one Annex-B access unit as memoryviews,
    start codes stripped. NAL type is `nal[0] & 0x1F`. Convenience wrapper
    around nal_bounds() - allocates, so keep it out of per-frame loops."""
    data = au if isinstance(au, (bytes, bytearray)) else bytes(au)
    bounds = array("I", bytes(64 * 4))
    mv = memoryview(data)
    for k in range(nal_bounds(data, bounds)):
        yield mv[bounds[k * 2]:bounds[k * 2 + 1]]


def _u16(v):
    return struct.pack(">H", v)


def _u32(v):
    return struct.pack(">I", v)


def _u64(v):
    return struct.pack(">Q", v)


def _box(tag, *payload):
    size = 8
    for p in payload:
        size += len(p)
    return b"".join((_u32(size), tag) + payload)


def _full(tag, version, flags, *payload):
    return _box(tag, struct.pack(">I", (version << 24) | flags), *payload)


_UNITY_MATRIX = (_u32(0x00010000) + bytes(12)
                 + _u32(0x00010000) + bytes(12) + _u32(0x40000000))


class MicSource:
    """Preallocated PCM ring buffer between the audio driver and a
    consumer (the MP4 recorder or the RTSP server). The caller owns the
    audio driver: pass callback to audio.start_streaming() and drain
    with read(). Neither side allocates in steady state."""

    def __init__(self, rate=16000, channels=1, ring=16384):
        self.rate = rate
        self.channels = channels
        self._buf = bytearray(ring)
        self._mv = memoryview(self._buf)
        self._head = 0
        self._tail = 0

    def callback(self, pcm):
        """Pass this to audio.start_streaming()."""
        mv = memoryview(pcm)
        n = len(mv)
        size = len(self._buf)
        pos = self._head
        first = min(n, size - pos)
        self._mv[pos:pos + first] = mv[:first]
        if first < n:
            self._mv[0:n - first] = mv[first:n]
        self._head = (pos + n) % size

    def settle(self, ms=300):
        """Call after audio.start_streaming(): waits out the microphone
        filters' start-up transient (a full-scale pop) and drops it."""
        import time
        time.sleep_ms(ms)
        self._tail = self._head

    def read(self):
        """One contiguous pending PCM chunk as a memoryview, or None."""
        head = self._head
        if self._tail == head:
            return None
        end = head if head > self._tail else len(self._buf)
        chunk = self._mv[self._tail:end]
        self._tail = end % len(self._buf)
        return chunk


class Mp4:
    def __init__(self, path, width, height, fps=30,
                 audio_rate=0, audio_channels=1, fragment_frames=60,
                 buffer_size=262144, audio_buffer=65536, sps_pps=None,
                 encoder=None, microphone=None):
        self.width = width
        self.height = height
        self.fps = fps
        self.audio_rate = audio_rate
        self.audio_channels = audio_channels
        self.fragment_frames = fragment_frames

        self._f = open(path, "wb") if isinstance(path, str) else path
        self._sps = None
        self._pps = None
        self._init_written = False
        self._seq = 0  # moof sequence number
        self._pts0 = None  # first video pts (us), zero point
        self._last_pts90 = 0  # monotonicity guard for tick wraparound
        self._frames = 0  # total video frames written
        self._audio_base = 0  # tfdt of the pending audio (sample frames)
        self._last_dur = _VIDEO_TIMESCALE // fps

        # All steady-state buffers are preallocated here; write() and
        # _flush() never grow or allocate them again.
        self._frag = bytearray(buffer_size)  # length-prefixed samples
        self._frag_mv = memoryview(self._frag)
        self._frag_len = 0
        self._scratch = bytearray(buffer_size)  # Annex-B copy for find()
        self._scratch_mv = memoryview(self._scratch)
        self._pcm = bytearray(audio_buffer)
        self._pcm_mv = memoryview(self._pcm)
        self._pcm_len = 0
        self._bounds = array("I", bytes(64 * 4))  # 32 NALs max
        self._sizes = array("I", bytes((fragment_frames + 1) * 4))
        self._keys = array("I", bytes((fragment_frames + 1) * 4))
        self._pts = array("I", bytes((fragment_frames + 1) * 4))
        self._nsamples = 0
        # moof scratch: mfhd(16) + video traf(8+16+20+20+12N) + audio
        # traf(8+24+20+20) + moof/mdat headers + slack.
        self._moof = bytearray(208 + 12 * (fragment_frames + 1))
        self._moof_mv = memoryview(self._moof)

        # A hardware encoder (e.g. codec.H264Encoder) may be attached so
        # its stream config and per-frame keyframe flag are used
        # automatically instead of being passed on every call.
        self._encoder = encoder

        # microphone= attaches a MicSource: the recording loop calls
        # poll_audio() and close() fades the tail out. The caller owns
        # the audio driver (audio.init/start_streaming(mic.callback)).
        self._mic = microphone
        if microphone is not None and not audio_rate:
            raise ValueError("audio_rate is 0")
        if sps_pps is None and encoder is not None:
            sps_pps = encoder.sps_pps()

        # Out-of-band codec config, e.g. a hardware encoder's sps_pps()
        # output, for streams that don't repeat SPS/PPS in-band.
        if sps_pps is not None:
            for nal in nal_units(sps_pps):
                ntype = nal[0] & 0x1F
                if ntype == 7:
                    self._sps = bytes(nal)
                elif ntype == 8:
                    self._pps = bytes(nal)

    # ------------------------------------------------------------------ #
    # Public API
    # ------------------------------------------------------------------ #
    def write(self, au, keyframe=None, timestamp_us=None):
        """Append one H.264 access unit (Annex-B bytes/bytearray/memoryview,
        or an object with .data/.keyframe/.timestamp_us attributes). With
        encoder= attached, keyframe defaults to encoder.keyframe()."""
        if isinstance(au, (bytes, bytearray, memoryview)):
            data = au
        else:
            data = au.data
            if keyframe is None:
                keyframe = au.keyframe
            if timestamp_us is None:
                timestamp_us = au.timestamp_us
        if timestamp_us is None:
            timestamp_us = self._frames * 1000000 // self.fps
        if self._pts0 is None:
            self._pts0 = timestamp_us
        pts90 = (timestamp_us - self._pts0) * 9 // 100
        # Non-monotonic input (e.g. time.ticks_us() wrapping after ~17.9
        # minutes) falls back to the nominal frame duration.
        if self._frames and pts90 <= self._last_pts90:
            pts90 = self._last_pts90 + _VIDEO_TIMESCALE // self.fps
        self._last_pts90 = pts90

        # find() needs bytes/bytearray; memoryviews (e.g. straight from a
        # hardware encoder) are copied once into the scratch buffer.
        if isinstance(data, memoryview):
            n = len(data)
            self._scratch[:n] = data
            src = self._scratch
            smv = self._scratch_mv
        else:
            n = len(data)
            src = data
            smv = memoryview(data)
        nals = nal_bounds(src, self._bounds, n)

        # First pass: sample size and keyframe detection (types: 7 SPS,
        # 8 PPS, 9 AUD are captured/dropped; 5 is an IDR slice).
        bounds = self._bounds
        size = 0
        has_idr = False
        k = 0
        while k < nals:  # while: a range object would allocate per frame
            begin = bounds[k * 2]
            end = bounds[k * 2 + 1]
            ntype = src[begin] & 0x1F
            if ntype == 7:
                self._sps = bytes(src[begin:end])
            elif ntype == 8:
                self._pps = bytes(src[begin:end])
            elif ntype != 9:
                size += 4 + end - begin
                if ntype == 5:
                    has_idr = True
            k += 1
        if keyframe is None:
            keyframe = self._encoder.keyframe() if self._encoder else has_idr
        if self._sps is None:
            raise ValueError("no SPS/PPS seen yet (pass sps_pps= or start with an IDR)")

        # Cut the pending fragment before a new keyframe, or at the size
        # caps. Flushing on entry means every duration is known exactly.
        if self._nsamples and (keyframe
                               or self._nsamples >= self.fragment_frames
                               or self._frag_len + size > len(self._frag)):
            self._flush(pts90)
        if self._frag_len + size > len(self._frag):
            raise ValueError("access unit larger than buffer_size")

        # Second pass: write length-prefixed NALs into the fragment.
        frag = self._frag
        pos = self._frag_len
        k = 0
        while k < nals:
            begin = bounds[k * 2]
            end = bounds[k * 2 + 1]
            ntype = src[begin] & 0x1F
            k += 1
            if ntype == 7 or ntype == 8 or ntype == 9:
                continue
            struct.pack_into(">I", frag, pos, end - begin)
            pos += 4
            # memoryview slice: copies the data without allocating a bytes
            # object for it (a bytes slice would copy the whole NAL twice).
            frag[pos:pos + end - begin] = smv[begin:end]
            pos += end - begin
        self._frag_len = pos

        i = self._nsamples
        self._sizes[i] = size
        self._keys[i] = 1 if keyframe else 0
        self._pts[i] = pts90
        self._nsamples = i + 1
        self._frames += 1

    def write_audio(self, pcm):
        """Append raw signed 16-bit little-endian PCM (audio_rate must be
        set). Interleaved into the file at the next fragment boundary."""
        if not self.audio_rate:
            raise ValueError("audio_rate is 0")
        n = len(pcm)
        if self._pcm_len + n > len(self._pcm):
            raise ValueError("audio_buffer overflow (fragment too long)")
        self._pcm[self._pcm_len:self._pcm_len + n] = pcm
        self._pcm_len += n

    def poll_audio(self):
        """Drain pending microphone audio into the file (microphone=True).
        Call once per video frame - the muxer copies each chunk."""
        chunk = self._mic.read()
        while chunk is not None:
            self.write_audio(chunk)
            chunk = self._mic.read()

    def _fade_tail(self):
        # Fade the pending audio out over ~30 ms, in place in the
        # preallocated buffer, so the recording does not end mid-waveform
        # with a click.
        n = self._pcm_len
        fade = min(n // 2, (self.audio_rate * self.audio_channels * 30) // 1000)
        base = n - fade * 2
        for i in range(fade):
            (v,) = struct.unpack_from("<h", self._pcm, base + i * 2)
            struct.pack_into("<h", self._pcm, base + i * 2,
                             (v * (fade - 1 - i)) // fade)

    def close(self):
        if self._f is None:
            return
        if self._mic is not None:
            import time
            time.sleep_ms(50)  # let the microphone deliver a final chunk
            self.poll_audio()
            self._mic = None
            self._fade_tail()
        if self._nsamples or self._pcm_len:
            last = self._pts[self._nsamples - 1] if self._nsamples else 0
            self._flush(last + self._last_dur)
        self._f.close()
        self._f = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

    # ------------------------------------------------------------------ #
    # Init segment (one-time; plain allocation is fine here)
    # ------------------------------------------------------------------ #
    def _write_init(self):
        ftyp = _box(b"ftyp", b"isom", _u32(0x200),
                    b"isom", b"iso2", b"iso5", b"avc1", b"mp41")

        traks = [self._trak_video()]
        trexs = [_full(b"trex", 0, 0, _u32(1), _u32(1), _u32(0), _u32(0), _u32(0))]
        if self.audio_rate:
            traks.append(self._trak_audio())
            trexs.append(_full(b"trex", 0, 0, _u32(2), _u32(1), _u32(0), _u32(0), _u32(0)))

        mvhd = _full(b"mvhd", 0, 0,
                     _u32(0), _u32(0),  # creation, modification
                     _u32(1000), _u32(0),  # timescale, duration (unknown)
                     _u32(0x00010000), _u16(0x0100), _u16(0),  # rate, volume
                     _u32(0), _u32(0), _UNITY_MATRIX,
                     bytes(24),  # pre_defined
                     _u32(len(traks) + 1))  # next_track_ID
        moov = _box(b"moov", mvhd, *(traks + [_box(b"mvex", *trexs)]))
        self._f.write(ftyp + moov)
        self._init_written = True

    def _trak_video(self):
        avcc = _box(b"avcC",
                    bytes((1, self._sps[1], self._sps[2], self._sps[3], 0xFF, 0xE1)),
                    _u16(len(self._sps)), self._sps,
                    b"\x01", _u16(len(self._pps)), self._pps)
        avc1 = _box(b"avc1",
                    bytes(6), _u16(1),  # reserved, data_reference_index
                    bytes(16),  # pre_defined/reserved
                    _u16(self.width), _u16(self.height),
                    _u32(0x00480000), _u32(0x00480000),  # 72 dpi
                    _u32(0), _u16(1),  # reserved, frame_count
                    bytes(32),  # compressorname
                    _u16(0x18), struct.pack(">h", -1),  # depth, pre_defined
                    avcc)
        stbl = _box(b"stbl",
                    _full(b"stsd", 0, 0, _u32(1), avc1),
                    _full(b"stts", 0, 0, _u32(0)),
                    _full(b"stsc", 0, 0, _u32(0)),
                    _full(b"stsz", 0, 0, _u32(0), _u32(0)),
                    _full(b"stco", 0, 0, _u32(0)))
        minf = _box(b"minf",
                    _full(b"vmhd", 0, 1, _u64(0)),
                    self._dinf(),
                    stbl)
        return self._trak(1, minf, b"vide", b"VideoHandler\x00",
                          _VIDEO_TIMESCALE, self.width, self.height, 0)

    def _trak_audio(self):
        sowt = _box(b"sowt",
                    bytes(6), _u16(1),  # reserved, data_reference_index
                    bytes(8),  # reserved
                    _u16(self.audio_channels), _u16(16),  # channels, bits
                    _u32(0),  # pre_defined/reserved
                    _u32(self.audio_rate << 16))
        stbl = _box(b"stbl",
                    _full(b"stsd", 0, 0, _u32(1), sowt),
                    _full(b"stts", 0, 0, _u32(0)),
                    _full(b"stsc", 0, 0, _u32(0)),
                    _full(b"stsz", 0, 0, _u32(0), _u32(0)),
                    _full(b"stco", 0, 0, _u32(0)))
        minf = _box(b"minf",
                    _full(b"smhd", 0, 0, _u32(0)),
                    self._dinf(),
                    stbl)
        return self._trak(2, minf, b"soun", b"SoundHandler\x00",
                          self.audio_rate, 0, 0, 0x0100)

    def _dinf(self):
        return _box(b"dinf", _full(b"dref", 0, 0, _u32(1),
                                   _full(b"url ", 0, 1)))  # data in this file

    def _trak(self, track_id, minf, handler, name, timescale, w, h, volume):
        tkhd = _full(b"tkhd", 0, 3,  # enabled + in movie
                     _u32(0), _u32(0), _u32(track_id), _u32(0), _u32(0),
                     _u64(0), _u16(0), _u16(0), _u16(volume), _u16(0),
                     _UNITY_MATRIX, _u32(w << 16), _u32(h << 16))
        mdhd = _full(b"mdhd", 0, 0,
                     _u32(0), _u32(0), _u32(timescale), _u32(0),
                     _u16(0x55C4), _u16(0))  # language "und"
        hdlr = _full(b"hdlr", 0, 0, _u32(0), handler, bytes(12), name)
        return _box(b"trak", tkhd, _box(b"mdia", mdhd, hdlr, minf))

    # ------------------------------------------------------------------ #
    # Fragments (steady state: struct.pack_into only, no allocation)
    # ------------------------------------------------------------------ #
    def _flush(self, next_pts90):
        if not self._init_written:
            self._write_init()
        self._seq += 1

        moof_len = self._build_moof(next_pts90)
        # mdat header (8 bytes) is packed into the moof scratch tail so the
        # moof+mdat header go out as one write.
        struct.pack_into(">I4s", self._moof, moof_len,
                         8 + self._frag_len + self._pcm_len, b"mdat")

        # A few large back-to-back sequential writes.
        self._f.write(self._moof_mv[:moof_len + 8])
        self._f.write(self._frag_mv[:self._frag_len])
        if self._pcm_len:
            self._f.write(self._pcm_mv[:self._pcm_len])

        if self._nsamples:
            self._last_dur = max(1, next_pts90 - self._pts[self._nsamples - 1])
        self._audio_base += self._pcm_len // (2 * self.audio_channels)
        self._frag_len = 0
        self._pcm_len = 0
        self._nsamples = 0

    def _build_moof(self, next_pts90):
        """Build moof into the preallocated scratch, return its length."""
        buf = self._moof
        ns = self._nsamples
        pcm_frames = self._pcm_len // (2 * self.audio_channels)

        # Sizes are fixed by the counts, so offsets are known up front.
        vtraf = (8 + 16 + 20 + (20 + 12 * ns)) if ns else 0
        atraf = (8 + 24 + 20 + 20) if pcm_frames else 0
        moof_len = 8 + 16 + vtraf + atraf
        video_off = moof_len + 8
        audio_off = video_off + self._frag_len

        struct.pack_into(">I4s", buf, 0, moof_len, b"moof")
        struct.pack_into(">I4sII", buf, 8, 16, b"mfhd", 0, self._seq)
        pos = 24

        if ns:
            struct.pack_into(">I4s", buf, pos, vtraf, b"traf")
            struct.pack_into(">I4sII", buf, pos + 8,  # tfhd: base is moof
                             16, b"tfhd", 0x020000, 1)
            struct.pack_into(">I4sIQ", buf, pos + 24,  # tfdt v1
                             20, b"tfdt", 0x01000000, self._pts[0])
            struct.pack_into(">I4sIIi", buf, pos + 44,  # trun: offset+dur+size+flags
                             20 + 12 * ns, b"trun", 0x701, ns, video_off)
            entry = pos + 64
            sizes = self._sizes
            keys = self._keys
            pts = self._pts
            i = 0
            while i < ns:
                dur = (pts[i + 1] if i + 1 < ns else next_pts90) - pts[i]
                struct.pack_into(">III", buf, entry,
                                 dur if dur > 0 else 1, sizes[i],
                                 _FLAG_KEY if keys[i] else _FLAG_NONKEY)
                entry += 12
                i += 1
            pos = entry

        if pcm_frames:
            struct.pack_into(">I4s", buf, pos, atraf, b"traf")
            struct.pack_into(">I4sIIII", buf, pos + 8,  # tfhd + default dur/size
                             24, b"tfhd", 0x020018, 2, 1, 2 * self.audio_channels)
            struct.pack_into(">I4sIQ", buf, pos + 32,  # tfdt v1
                             20, b"tfdt", 0x01000000, self._audio_base)
            struct.pack_into(">I4sIIi", buf, pos + 52,  # trun: offset only
                             20, b"trun", 0x1, pcm_frames, audio_off)
            pos += atraf

        return moof_len

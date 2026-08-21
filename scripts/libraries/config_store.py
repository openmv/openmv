import os, struct
import vfs, mimxrt
try:
    from binascii import crc32
except ImportError:
    print("Error: crc32 not found, using custom implementation")
    def crc32(data, crc=0):
        crc ^= 0xFFFFFFFF
        for b in data:
            crc ^= b
            for _ in range(8):
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1))
        return crc ^ 0xFFFFFFFF

_MAGIC = b'ABST'
_HDR = 16  # magic(4) + seq(4) + len(4) + crc(4)

class ConfigStore:
    def __init__(self, path='/flash', name='machinestate', max_bytes=100000): # 100KB
        self.slot = _HDR + max_bytes
        self.max_bytes = max_bytes
        self.files = (f'{path}/{name}_a', f'{path}/{name}_b')
        self._ensure()

    def _ensure(self):
        # Only to check flash is mounted or not
        try:
            os.chdir("/flash")
            print("/flash accessible !!")
        except OSError:  
            # /flash not mounted — mount it manually
            print("Error: /flash not mounted — mounting it manually")          
            try:
                vfs.mount(vfs.VfsFat(mimxrt.Flash()), "/flash")
                print("/flash mounted successfully")
            except OSError as e:
                print(f"OSError: in mounting /flash manually, {str(e)}")

            os.chdir("/flash")
        
        # The ONLY place files are created/sized. After this, in-place writes only.
        try:
            for f in self.files:
                try:
                    if os.stat(f)[6] == self.slot:
                        continue
                except OSError:
                    pass
                with open(f, 'wb') as fh:
                    fh.write(b'\x00' * self.slot); fh.flush()
        except Exception as e:
            print(f"Exception: in ensuring files, {str(e)}")

    def _read(self, f):
        try:
            with open(f, 'rb') as fh:
                raw = fh.read(self.slot)
        except OSError:
            return None
        if len(raw) < _HDR or raw[:4] != _MAGIC:
            return None
        seq, ln, want = struct.unpack('<III', raw[4:16])
        if ln > self.max_bytes:
            return None
        payload = raw[_HDR:_HDR + ln]
        if crc32(payload) & 0xFFFFFFFF != want:
            return None
        return seq, payload

    def load(self):
        best = None
        for f in self.files:
            r = self._read(f)
            if r and (best is None or r[0] > best[0]):
                best = r
        return best[1] if best else None      # bytes, or None if never saved

    def save(self, data):
        if len(data) > self.max_bytes:
            raise ValueError(f'payload too large: {len(data)} bytes > {self.max_bytes} bytes')
        seqs = [(-1 if r is None else r[0]) for r in (self._read(f) for f in self.files)]
        target = 0 if seqs[0] <= seqs[1] else 1     # overwrite the OLDER slot
        rec = struct.pack('<4sIII', _MAGIC, (max(seqs) + 1) & 0xFFFFFFFF,
                          len(data), crc32(data) & 0xFFFFFFFF) + bytes(data)
        rec += b'\x00' * (self.slot - len(rec))
        with open(self.files[target], 'r+b') as fh:
            fh.seek(0); fh.write(rec); fh.flush()
        try: os.sync()
        except (AttributeError, OSError): pass

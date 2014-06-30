import struct

class AVI:
    def __init__(self, path, w, h, fps, codec="MJPG"):
        self.w = w
        self.h = h
        self.fps = fps
        self.codec = codec
        self.size   = 0
        self.frames = 0
        self.fp = open(path, "w")
        self.fp.seek(224) #skip headers

    def avi_hdr(self):
        hdr  = struct.pack("I", int(1000/self.fps)) # Time delay between frames
        hdr += struct.pack("I", 0)          # Data rate of AVI data
        hdr += struct.pack("I", 1)          # Size of single unit of padding
        hdr += struct.pack("I", 0)          # Flags
        hdr += struct.pack("I", self.frames)# Number of video frame stored
        hdr += struct.pack("I", 0)          # Number of intial frames
        hdr += struct.pack("I", 1)          # Number of data streams in chunk
        hdr += struct.pack("I", 0)          # Minimum playback buffer size
        hdr += struct.pack("I", self.w)     # Width of video frame in pixels
        hdr += struct.pack("I", self.h)     # Height of video frame in pixels
        hdr += struct.pack("I", self.fps)   # Time scale
        hdr += struct.pack("I", 0)          # Data rate of playback
        hdr += struct.pack("I", 0)          # Starting time of AVI data
        hdr += struct.pack("I", 0)          # Size of AVI data chunk
        return hdr;

    def str_hdr(self):
        hdr  = struct.pack("4s", "vids")    # Stream type
        hdr += struct.pack("4s", self.codec)# Stream codec
        hdr += struct.pack("I", 0)        # Flags
        hdr += struct.pack("I", 0)        # Priority
        hdr += struct.pack("I", 0)        # Number of first frame
        hdr += struct.pack("I", self.fps) # Time scale
        hdr += struct.pack("I", self.frames) # Data rate of playback
        hdr += struct.pack("I", 0)        # Starting time of AVI data
        hdr += struct.pack("I", 0)        # Data length
        hdr += struct.pack("I", 0)        # Buffer size
        hdr += struct.pack("I", 0)        # Sample quailty factor
        hdr += struct.pack("I", 0)        # Size of the sample in bytes
        hdr += struct.pack("II",0,0)      # Rect
        return hdr;

    def str_fmt(self):
        #BITMAPINFOHEADER
        hdr  = struct.pack("I", 40)       # Size in bytes
        hdr += struct.pack("I", self.w)   # Width
        hdr += struct.pack("I", self.h)   # Height
        hdr += struct.pack("H", 1)        # Planes
        hdr += struct.pack("H", 16)       # Bits per pixel
        hdr += struct.pack("4s", self.codec) # This should be BI_JPEG, but ffmpeg writes "MJPG"
        hdr += struct.pack("I", 0)        # Image size (which one?)
        hdr += struct.pack("I", 0)        # X pixels-per-meter
        hdr += struct.pack("I", 0)        # Y pixels-per-meter
        hdr += struct.pack("I", 0)        # color indexes in the color table
        hdr += struct.pack("I", 0)        # required color indexes in the color table
        return hdr;

    def new_chunk(self, c_id, c_data):
        return  c_id +\
                struct.pack("I", len(c_data)) +\
                c_data

    def new_list(self, l_id, l_4cc, l_size, l_data):
        return  l_id +\
                struct.pack("I",  l_size+len(l_data)+4) +\
                struct.pack("4s", l_4cc) +\
                l_data

    def add_frame(self, img):
        self.frames +=1
        self.size += img.size()
        self.fp.write(struct.pack("4sI", "00dc", img.size()))
        self.fp.write(img)

    def flush(self):
        self.fp.seek(0)
        self.fp.write(
            self.new_list(b"RIFF", b"AVI ", self.size,
                self.new_list(b"LIST", b"hdrl", 0,
                    self.new_chunk(b"avih", self.avi_hdr())
                    + self.new_list(b"LIST", b"strl", 0,
                          self.new_chunk(b"strh", self.str_hdr())
                        + self.new_chunk(b"strf", self.str_fmt())
                     )
                    + self.new_list(b"LIST", b"movi", self.size, b"")
                )
            )
        )
        self.fp.close()

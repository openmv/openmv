import image, audio, time
from ulab import numpy as np
from ulab import scipy as sp
from ulab import utils

CHANNELS = 1
FREQUENCY = 32000
N_SAMPLES = 32 if FREQUENCY == 16000 else 64
SCALE = 1
SIZE = (N_SAMPLES * SCALE) // CHANNELS

raw_buf = None
fb = image.Image(SIZE+(50*SCALE), SIZE, image.RGB565, copy_to_fb=True)
audio.init(channels=CHANNELS, frequency=FREQUENCY, gain_db=16, overflow=False)

def audio_callback(buf):
    # NOTE: do Not call any function that allocates memory.
    global raw_buf
    if (raw_buf == None):
        raw_buf = buf

# Start audio streaming
audio.start_streaming(audio_callback)

def draw_fft(img, fft_buf):
    fft_buf = (fft_buf / max(fft_buf)) * SIZE
    fft_buf = np.log10(fft_buf + 1) * 20
    color = (0xFF, 0x0F, 0x00)
    for i in range(0, len(fft_buf)):
        img.draw_line(i*SCALE, SIZE, i*SCALE, SIZE-int(fft_buf[i]) * SCALE, color, SCALE)

def draw_audio_bar(img, level, offset):
    blk_size = (SIZE//10)
    color = (0xFF, 0x00, 0xF0)
    blk_space = (blk_size//4)
    for i in range(0, int(round(level/10))):
        fb.draw_rectangle(SIZE+offset, SIZE - ((i+1)*blk_size) + blk_space, 20 * SCALE, blk_size - blk_space, color, 1, True)

while (True):
    if (raw_buf != None):
        pcm_buf = np.frombuffer(raw_buf, dtype=np.int16)
        raw_buf = None

        if CHANNELS == 1:
            fft_buf = utils.spectrogram(pcm_buf)
            l_lvl = int((np.mean(abs(pcm_buf[1::2])) / 32768)*100)
        else:
            fft_buf = utils.spectrogram(pcm_buf[0::2])
            l_lvl = int((np.mean(abs(pcm_buf[1::2])) / 32768)*100)
            r_lvl = int((np.mean(abs(pcm_buf[0::2])) / 32768)*100)

        fb.clear()
        draw_fft(fb, fft_buf)
        draw_audio_bar(fb, l_lvl, 0)
        draw_audio_bar(fb, l_lvl, 25*SCALE)
        if CHANNELS == 2:
            draw_audio_bar(fb, r_lvl, 25 * SCALE)
        fb.flush()

# Stop streaming
audio.stop_streaming()

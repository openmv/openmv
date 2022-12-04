# MicroSpeech demo.
#
# Download the pre-trained Yes/No model from here:
# https://raw.githubusercontent.com/iabdalkader/microspeech-yesno-model/main/model.tflite
# Save the model to storage, reset and run the example.
import audio, time, tf, micro_speech, pyb
labels = ['Silence', 'Unknown', 'Yes', 'No']

led_red   = pyb.LED(1)
led_green = pyb.LED(2)

model = tf.load('/model.tflite')
speech = micro_speech.MicroSpeech()
audio.init(channels=1, frequency=16000, gain=24, highpass=0.9883)

# Start audio streaming
audio.start_streaming(speech.audio_callback)

while (True):
    # Run micro-speech without a timeout and filter detections by label index.
    idx = speech.listen(model, timeout=0, threshold = 0.78, filter=[2, 3])
    led = led_green if idx == 2 else led_red
    print(labels[idx])
    for i in range(0, 4):
        led.on(); time.sleep_ms(25)
        led.off(); time.sleep_ms(25)

# Stop streaming
audio.stop_streaming()

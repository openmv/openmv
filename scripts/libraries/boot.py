import sensor, image, pyb, machine, time, sys, binascii, os, ml, uos, gc
from machine import UART
# Initialize LED PINs
RED_LED_PIN = 1
GREEN_LED_PIN = 2
BLUE_LED_PIN = 3
pyb.freq(400000000)
good_image = 0
# Initialize UART
uart = UART(3, 115200)
uart.init(115200, bits=8, parity=None, stop=1)

# Initialize sensor
sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)

def clear_uart_buffer(uart):
    """Clear any remaining data in the UART buffer."""
    while uart.any():
        uart.read()

def capture_image():
    pyb.LED(BLUE_LED_PIN).on()
    img = sensor.snapshot()
    picture = 'pic.jpg'
    img.save(picture)


    with open(picture, "rb") as f:
        image_data = f.read()
        image_size = len(image_data)
        length = str(image_size) + '\n'
        print("image size:", length)

    # Send image data in hex format
    with open(picture, "rb") as f:
        content = f.read()
        hex_content = binascii.hexlify(content)
        print("Hex representation of image data:", hex_content)
    pyb.LED(BLUE_LED_PIN).off()
    return img
def classify_image(img):
    global good_image
    global bad_image
    net = None
    labels = None

    try:
        net = ml.Model("trained160.tflite", load_to_fb=True)
    except Exception as e:
        print("Failed to load model:", e)

    try:
        labels = [line.rstrip('\n') for line in open("labels.txt")]
    except Exception as e:
        print("Failed to load labels:", e)

    try:
        prediction = net.predict([img])  # Predict the class
        output = prediction[0].flatten()  # Flatten the output to get confidence scores

        # Print the results with confidence scores
        print("Prediction:")
        for i, label in enumerate(labels):
            print(f"{label}: {output[i]:.2f}")

        # Find the class with the highest confidence score
        best_label_index = max(range(len(output)), key=lambda i: output[i])
        best_label = labels[best_label_index]
        print(f"Classified as: {best_label}")

        classified_image = f"{best_label}_image.jpg"
        img.save(classified_image)
        print(f"Image saved as: {classified_image}")
        indicator = "GOOD" if best_label == "good" else "BAD"

        if best_label == "bad":
            send_image_via_uart(classified_image, "bad")
            print(f"{classified_image} sent via UART with indicator 'bad'")

        if best_label == "good":
            send_image_via_uart(classified_image, "good")
            print(f"{classified_image} sent via UART with indicator 'good'")
    except Exception as e:
        print("Error in prediction:", e)

def send_image_via_uart(image_path, indicator):
    with open(image_path, "rb") as f:
            img_data = f.read()
            img_size = len(img_data)
            indicator = (indicator.upper() + "00000")[:5]
#            indicator = indicator.ljust(5, '0')
            length = f"{img_size}:{indicator}\n"
            uart.write(length.encode())
            print(f"Size and indicator sent: {length.strip()}")
            uart.write(img_data)
            print(f"{image_path} sent via UART with indicator {indicator}")
while True:
    img = capture_image()
    classify_image(img)  
    time.sleep(3)

#     break
#    command = uart.read(5)

#    if command is None:
#        continue

#    print("Received command:", command)

#    if command == b"CAMON":
#        clear_uart_buffer(uart)
#        print("Camera ON, starting image capture.")
#        img = capture_image()
#        classify_image(img)

##        pyb.delay(20000)
#        print("Image transmission completed")
#    time.sleep(1)



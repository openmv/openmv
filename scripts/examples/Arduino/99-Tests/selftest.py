# Self Test Example
#
# This example shows how your OpenMV Cam tests itself before being allowed out
# of the factory. Every OpenMV Cam should pass this test.

import sensor, time, pyb

def test_int_adc():
    adc  = pyb.ADCAll(12)
    # Test VBAT
    vbat = adc.read_core_vbat()
    vbat_diff = abs(vbat-3.3)
    if (vbat_diff > 0.1):
        raise Exception('INTERNAL ADC TEST FAILED VBAT=%fv'%vbat)

    # Test VREF
    vref = adc.read_core_vref()
    vref_diff = abs(vref-1.2)
    if (vref_diff > 0.1):
        raise Exception('INTERNAL ADC TEST FAILED VREF=%fv'%vref)
    adc = None
    print('INTERNAL ADC TEST PASSED...')

def test_color_bars():
    sensor.reset()
    # Set sensor settings
    sensor.set_brightness(0)
    sensor.set_saturation(3)
    sensor.set_gainceiling(8)
    sensor.set_contrast(2)

    # Set sensor pixel format
    sensor.set_framesize(sensor.QVGA)
    sensor.set_pixformat(sensor.RGB565)

    # Enable colorbar test mode
    sensor.set_colorbar(True)

    # Skip a few frames to allow the sensor settle down
    for i in range(0, 100):
        image = sensor.snapshot()

    #color bars thresholds
    t = [lambda r, g, b: r < 70  and g < 70  and b < 70,   # Black
         lambda r, g, b: r < 70  and g < 70  and b > 200,  # Blue
         lambda r, g, b: r > 200 and g < 70  and b < 70,   # Red
         lambda r, g, b: r > 200 and g < 70  and b > 200,  # Purple
         lambda r, g, b: r < 70  and g > 200 and b < 70,   # Green
         lambda r, g, b: r < 70  and g > 200 and b > 200,  # Aqua
         lambda r, g, b: r > 200 and g > 200 and b < 70,   # Yellow
         lambda r, g, b: r > 200 and g > 200 and b > 200]  # White

    # color bars are inverted for OV7725
    if (sensor.get_id() == sensor.OV7725):
        t = t[::-1]

    #320x240 image with 8 color bars each one is approx 40 pixels.
    #we start from the center of the frame buffer, and average the
    #values of 10 sample pixels from the center of each color bar.
    for i in range(0, 8):
        avg = (0, 0, 0)
        idx = 40*i+20 #center of colorbars
        for off in range(0, 10): #avg 10 pixels
            rgb = image.get_pixel(idx+off, 120)
            avg = tuple(map(sum, zip(avg, rgb)))

        if not t[i](avg[0]/10, avg[1]/10, avg[2]/10):
            raise Exception('COLOR BARS TEST FAILED.'
            'BAR#(%d): RGB(%d,%d,%d)'%(i+1, avg[0]/10, avg[1]/10, avg[2]/10))

    print('COLOR BARS TEST PASSED...')

if __name__ == '__main__':
    print('')
    test_int_adc()
    test_color_bars()


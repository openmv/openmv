from machine import I2C
import time

i2c = I2C(2)
imu = BNO055(i2c)
while True:
    # temperature = imu.temperature()
    # print('temperature: ℃',temperature)
    yaw, roll, pitch = imu.euler()
    print("yaw, roll, pitch: °", yaw, roll, pitch)
    # w, x, y, z = imu.quaternion()
    # print('Quaternion:', w, x, y, z)
    # x, y, z = imu.accelerometer()
    # print('Accelerometer (m/s^2):', x, y, z)
    # x, y, z = imu.magnetometer()
    # print('Magnetometer (microteslas):', x, y, z)
    # x, y, z = imu.gyroscope()
    # print('Gyroscope (deg/sec):', x, y, z)
    # x, y, z = imu.linear_acceleration()
    # print('Linear acceleration (m/s^2)', x, y, z)
    # x, y, z = imu.gravity()
    # print('Gravity (m/s^2):', x, y, z)
    time.sleep_ms(100)

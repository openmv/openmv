import time, struct#, network
from machine import UART

# SSID = "Elmer Fudd"
# KEY = "fy9ifeatrn"

# # Init wlan module
# wlan = network.WLAN(network.STA_IF)
# wlan.active(True)

# # Connect to wifi
# wlan.connect(SSID, KEY)

# # Wait for connection
# while True:
#     if wlan.isconnected():
#         print("connected")
#         break
#     print("Waiting for connection")
#     time.sleep(1)

# for _ in range(10):
#     time.sleep(1)
# uart = UART(1, baudrate=115200, timeout_char=1000)
# MOVES = [
#     (-500, 0, 500, 0),   # CMD_MOVE_LEFT
#     (500,  0, 500, 0),   # CMD_MOVE_RIGHT
#     (0,    0, 500, 0),   # CMD_MOVE_UP
#     (0,    0, -500, 0),  # CMD_MOVE_DOWN
#     (0,    0, 0, 0)      # CMD_STOP
# ]
# def send_packet(vx, vy, vz, yaw):
#     packet = struct.pack('<Bhhhh', ord('s'), vx, vy, vz, yaw)
#     uart.write(packet)
#     print("Sent packet:", [hex(b) for b in bytearray(packet)])

# # Infinite loop sending commands every second
# while True:
#     for vx, vy, vz, yaw in MOVES:
#         send_packet(vx, vy, vz, yaw)
#         time.sleep(1.0)

uart = UART(1, baudrate=115200, timeout_char=1000)
MOVES = [
    (-500, 0, 500, 0),   # CMD_MOVE_LEFT
    (500,  0, 500, 0),   # CMD_MOVE_RIGHT
    (0,    0, 500, 0),   # CMD_MOVE_UP
    (0,    0, -500, 0),  # CMD_MOVE_DOWN
    (0,    0, 0, 0)      # CMD_STOP
]
def send_packet(vx, vy, vz, yaw):
    packet = struct.pack('<Bhhhh', ord('s'), vx, vy, vz, yaw)
    uart.write(packet)
    print("Sent packet:", [hex(b) for b in bytearray(packet)])

# Infinite loop sending commands every second
# while True:
#     for vx, vy, vz, yaw in MOVES:
#         send_packet(vx, vy, vz, yaw)
#         time.sleep(1.0)
time.sleep(5.0)
start = time.ticks_ms()
while time.ticks_diff(time.ticks_ms(), start) < 10000:  # 10000 ms = 10 sec
    send_packet(0, 0, 1000, 0)
    time.sleep_ms(20)

send_packet(0, 0, 0, 0)
    

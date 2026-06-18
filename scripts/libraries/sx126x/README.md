# SX126x LoRa Driver

Python driver for Semtech SX1261/SX1262/SX1268 LoRa transceivers, ported from
[micropySX126X](https://github.com/ehong-tl/micropySX126X) (originally based on
[RadioLib](https://github.com/jgromes/RadioLib)).

## Supported Modules

- SX1262
- SX1268

(SX1261 is also included for upstream parity.)

## Tested Boards

- OpenMV RT1062 (`OPENMV_RT1060` firmware target)

## Required Connections (OpenMV RT1062)

Wire your SX1262/SX1268 breakout to the OpenMV RT1062 header as follows:

| SX126x Signal | OpenMV Pin | Notes |
|---------------|------------|-------|
| SCK           | P2         | Via `SPI(1)` — auto-muxed |
| MOSI          | P0         | Via `SPI(1)` |
| MISO          | P1         | Via `SPI(1)` |
| NSS / CS      | P3         | Manual GPIO chip select |
| BUSY          | P4         | Any free GPIO (example default) |
| RESET         | P5         | Any free GPIO (example default) |
| DIO1 (IRQ)    | P7         | IRQ-capable GPIO (example default) |

Avoid P6 (ADC). P8–P14 may be used by shields or camera peripherals depending
on your setup.

## Example Usage

```python
from machine import SPI
from sx1262 import SX1262

spi = SPI(1, baudrate=2_000_000, polarity=0, phase=0)
sx = SX1262(1, 0, 0, 0, "P3", "P7", "P5", "P4", spi=spi)
sx.begin(freq=915.0, bw=125.0, sf=9, cr=7, syncWord=0x12, power=14)

length, status = sx.send(b"Hello OpenMV")
print("TX:", length, status)

msg, status = sx.recv()
print("RX:", msg, "RSSI:", sx.getRSSI(), "SNR:", sx.getSNR())
```

See `scripts/examples/50-OpenMV-Boards/50-IMXRT-Boards/52-LoRa/` for complete
sender, receiver, and ping-pong examples.

## Notes

- The driver is frozen into `OPENMV_RT1060` firmware. Import with
  `from sx1262 import SX1262` (or `sx1268`).
- TX and RX devices must use matching radio parameters (frequency, bandwidth,
  spreading factor, coding rate, sync word).
- This driver provides raw LoRa packet radio. It is unrelated to the UART-based
  LoRaWAN `lora` library used on Arduino Portenta boards.

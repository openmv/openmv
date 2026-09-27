// NUCLEO-N657X0-Q pin definitions (see board_config.h).

// SPI5: Arduino D10 (NSS), D11 (MOSI), D12 (MISO), D13 (SCK)
OMV_GPIO_DEFINE(E, 15, AF5, SPI5)       // SPI5_SCK
OMV_GPIO_DEFINE(G, 1,  AF5, SPI5)       // SPI5_MISO
OMV_GPIO_DEFINE(G, 2,  AF5, SPI5)       // SPI5_MOSI
OMV_GPIO_DEFINE(A, 3,  AF5, SPI5)       // SPI5_NSS

// I2C1: Arduino D15 (SCL), D14 (SDA)
OMV_GPIO_DEFINE(H, 9,  AF4, I2C1)       // I2C1_SCL
OMV_GPIO_DEFINE(C, 1,  AF4, I2C1)       // I2C1_SDA

// I2C2: camera connector
OMV_GPIO_DEFINE(B, 10, AF4, I2C2)       // I2C2_SCL
OMV_GPIO_DEFINE(B, 11, AF4, I2C2)       // I2C2_SDA

// I2C4
OMV_GPIO_DEFINE(E, 13, AF4, I2C4)       // I2C4_SCL
OMV_GPIO_DEFINE(E, 14, AF4, I2C4)       // I2C4_SDA

// Camera connector control lines
OMV_GPIO_DEFINE(O, 5,  NONE, GPIO)      // CSI_RESET (NRST_CAM, active low, VDDIO2 3.3V)
OMV_GPIO_DEFINE(A, 0,  NONE, GPIO)      // CSI_POWER (EN_CAM, high = on)

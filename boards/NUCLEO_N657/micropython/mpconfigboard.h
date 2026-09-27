// MicroPython board configuration for OpenMV firmware on the ST NUCLEO-N657X0-Q.
//
// Derived from the OPENMV_N6 and NUCLEO_N657X0 MicroPython boards. The Nucleo has no
// external RAM, no WiFi/BT module and no SD card; it has RMII Ethernet, three user
// LEDs, one user button and the 512 Mbit MX25UM51245G octo-SPI flash on XSPI2.

#define MICROPY_HW_BOARD_NAME       "NUCLEO-N657X0-Q"
#define MICROPY_HW_MCU_NAME         "STM32N657X0"

#define MICROPY_GC_STACK_ENTRY_TYPE uint32_t
#define MICROPY_ALLOC_GC_STACK_SIZE (128)

#define MICROPY_OBJ_REPR            (MICROPY_OBJ_REPR_C)

#define MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE (0)
#define MICROPY_HW_HAS_SWITCH       (1)
#define MICROPY_HW_HAS_FLASH        (1)
#define MICROPY_HW_ENABLE_RNG       (1)
#define MICROPY_HW_ENABLE_RTC       (1)
#define MICROPY_HW_ENABLE_DAC       (0)
#define MICROPY_HW_ENABLE_USB       (1)
#define MICROPY_HW_ENABLE_SDCARD    (0)
#define MICROPY_PY_PYB_LEGACY       (0)
#define MICROPY_FATFS_EXFAT         (1)

#define MICROPY_BOARD_ENTER_BOOTLOADER board_enter_bootloader
#define MICROPY_BOARD_EARLY_INIT    board_early_init
#define MICROPY_BOARD_ENTER_STANDBY board_enter_standby();
#define MICROPY_BOARD_LEAVE_STANDBY board_leave_standby();

// HSE is 48MHz (same as the OpenMV N6), this gives a CPU frequency of 800MHz.
#define MICROPY_HW_CLK_PLLM         (6)
#define MICROPY_HW_CLK_PLLN         (100)
#define MICROPY_HW_CLK_PLLP1        (1)
#define MICROPY_HW_CLK_PLLP2        (1)
#define MICROPY_HW_CLK_PLLFRAC      (0)

// The LSE is a 32kHz crystal.
#define MICROPY_HW_RTC_USE_LSE      (1)
#define MICROPY_HW_RTC_USE_US       (1)

// External SPI flash, MX25UM51245GXDI00 (512Mbit). Only the first 32MB are used by
// the OpenMV partition layout (bootloader, firmware, filesystem, ROMFS).
#define MICROPY_HW_XSPIFLASH_SIZE_BITS_LOG2 (29)

// ROMFS config
#define MICROPY_HW_ROMFS_ENABLE_EXTERNAL_XSPI (1)
#define MICROPY_HW_ROMFS_XSPI_SPIBDEV_OBJ (&spi_bdev)
#define MICROPY_HW_ROMFS_ENABLE_PART0 (1)

// SPI flash, block device config (same 4MB filesystem window as the OpenMV N6).
#define MICROPY_HW_BDEV_SPIFLASH                (&spi_bdev)
#define MICROPY_HW_BDEV_SPIFLASH_EXTENDED       (&spi_bdev)
#define MICROPY_HW_BDEV_SPIFLASH_CONFIG         (&spiflash_config)
#define MICROPY_HW_BDEV_SPIFLASH_OFFSET_BYTES   (4 * 1024 * 1024)
#define MICROPY_HW_BDEV_SPIFLASH_SIZE_BYTES     (4 * 1024 * 1024)

// UART buses. UART1 is the ST-LINK virtual COM port.
#define MICROPY_HW_UART1_TX         (pyb_pin_UART1_TX)
#define MICROPY_HW_UART1_RX         (pyb_pin_UART1_RX)
#define MICROPY_HW_UART3_TX         (pyb_pin_UART3_TX)
#define MICROPY_HW_UART3_RX         (pyb_pin_UART3_RX)

// I2C buses. I2C1 = Arduino D14/D15, I2C2 = camera connector, I2C4 = PE13/PE14.
#define MICROPY_HW_I2C1_SCL         (pyb_pin_I2C1_SCL)
#define MICROPY_HW_I2C1_SDA         (pyb_pin_I2C1_SDA)
#define MICROPY_HW_I2C2_SCL         (pyb_pin_I2C2_SCL)
#define MICROPY_HW_I2C2_SDA         (pyb_pin_I2C2_SDA)
#define MICROPY_HW_I2C4_SCL         (pyb_pin_I2C4_SCL)
#define MICROPY_HW_I2C4_SDA         (pyb_pin_I2C4_SDA)

// SPI buses. SPI5 = Arduino D10-D13.
#define MICROPY_HW_SPI5_NSS         (pyb_pin_SPI5_CS)
#define MICROPY_HW_SPI5_SCK         (pyb_pin_SPI5_SCK)
#define MICROPY_HW_SPI5_MISO        (pyb_pin_SPI5_MISO)
#define MICROPY_HW_SPI5_MOSI        (pyb_pin_SPI5_MOSI)

// USER button is floating, and pressing it makes the input go high.
#define MICROPY_HW_USRSW_PIN        (pyb_pin_BUTTON)
#define MICROPY_HW_USRSW_PULL       (GPIO_PULLDOWN)
#define MICROPY_HW_USRSW_EXTI_MODE  (GPIO_MODE_IT_RISING)
#define MICROPY_HW_USRSW_PRESSED    (1)

// LEDs (active low).
#define MICROPY_HW_LED1             (pyb_pin_LED_RED)
#define MICROPY_HW_LED2             (pyb_pin_LED_GREEN)
#define MICROPY_HW_LED3             (pyb_pin_LED_BLUE)
#define MICROPY_HW_LED_ON(pin)      (mp_hal_pin_low(pin))
#define MICROPY_HW_LED_OFF(pin)     (mp_hal_pin_high(pin))

// USB config. OpenMV's VID/PID so that OpenMV IDE recognises the board.
#define MICROPY_HW_USB_HS           (1)
#define MICROPY_HW_USB_HS_IN_FS     (1)
#define MICROPY_HW_USB_MAIN_DEV     (USB_PHY_HS_ID)
#define MICROPY_HW_USB_VID          0x37C5
#define MICROPY_HW_USB_PID          0x1206
#define MICROPY_HW_USB_PID_CDC      (MICROPY_HW_USB_PID)
#define MICROPY_HW_USB_PID_MSC      (MICROPY_HW_USB_PID)
#define MICROPY_HW_USB_PID_CDC_MSC  (MICROPY_HW_USB_PID)
#define MICROPY_HW_USB_PID_CDC_HID  (MICROPY_HW_USB_PID)
#define MICROPY_HW_USB_PID_CDC_MSC_HID  (MICROPY_HW_USB_PID)

// Ethernet via RMII (on-board PHY, same pins as the MicroPython NUCLEO_N657X0 board).
#define MICROPY_HW_ETH_MDC                      (pin_G11)
#define MICROPY_HW_ETH_MDIO                     (pin_F4)
#define MICROPY_HW_ETH_RMII_REF_CLK             (pin_F7)
#define MICROPY_HW_ETH_RMII_CRS_DV              (pin_F10)
#define MICROPY_HW_ETH_RMII_RXD0                (pin_F14)
#define MICROPY_HW_ETH_RMII_RXD1                (pin_F15)
#define MICROPY_HW_ETH_RMII_TX_EN               (pin_F11)
#define MICROPY_HW_ETH_RMII_TXD0                (pin_F12)
#define MICROPY_HW_ETH_RMII_TXD1                (pin_F13)

// USB CDC config
#define CFG_TUD_CDC_EP_BUFSIZE  (4096)
#define CFG_TUD_CDC_RX_BUFSIZE  (4096)
#define CFG_TUD_CDC_TX_BUFSIZE  (4096)

/******************************************************************************/
// Bootloader configuration (only used if mboot is built instead of OpenMV's bootloader)

#define MBOOT_BOARD_EARLY_INIT(initial_r0)      mboot_board_early_init()

#define MBOOT_SPIFLASH_CS                       (pyb_pin_XSPIM_P2_CS)
#define MBOOT_SPIFLASH_SCK                      (pyb_pin_XSPIM_P2_SCK)
#define MBOOT_SPIFLASH_MOSI                     (pyb_pin_XSPIM_P2_IO0)
#define MBOOT_SPIFLASH_MISO                     (pyb_pin_XSPIM_P2_IO1)
#define MBOOT_SPIFLASH_ADDR                     (0x70000000)
#define MBOOT_SPIFLASH_BYTE_SIZE                (32 * 1024 * 1024)
#define MBOOT_SPIFLASH_LAYOUT                   "/0x70000000/8192*4Kg"
#define MBOOT_SPIFLASH_ERASE_BLOCKS_PER_PAGE    (1)
#define MBOOT_SPIFLASH_SPIFLASH                 (&spi_bdev.spiflash)
#define MBOOT_SPIFLASH_CONFIG                   (&spiflash_config)

/******************************************************************************/
// Function and variable declarations

extern const struct _mp_spiflash_config_t spiflash_config;
extern struct _spi_bdev_t spi_bdev;

void mboot_board_early_init(void);
void mboot_board_entry_init(void);

void board_enter_bootloader(unsigned int n_args, const void *args);
void board_early_init(void);
void board_enter_standby(void);
void board_leave_standby(void);

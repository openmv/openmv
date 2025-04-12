/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ALif OSPI driver.
 * Note this driver uses SPI mode, and should support any standard flash.
 */
#include <stdint.h>
#include <string.h>
#include ALIF_CMSIS_H
#include "omv_boardconfig.h"
#include "omv_bootconfig.h"

#if OMV_BOOT_SPI_FLASH_ENABLE
#include "gpio.h"
#include "pinconf.h"
#include "ospi_private.h"

#define OSPI_INST_0BIT      (0x00U)
#define OSPI_INST_4BIT      (0x01U)
#define OSPI_INST_8BIT      (0x02U)
#define OSPI_INST_16BIT     (0x03U)

#define OSPI_DFS_8BIT       (0x07U)
#define OSPI_DFS_16BIT      (0x0FU)
#define OSPI_DFS_32BIT      (0x1FU)

#define OSPI_FRF_SINGLE     (0x00U)
#define OSPI_FRF_DUAL       (0x01U)
#define OSPI_FRF_QUAD       (0x02U)
#define OSPI_FRF_OCTAL      (0x03U)

#define OSPI_ADDR_0BIT      (0x00U)
#define OSPI_ADDR_24BIT     (0x06U)
#define OSPI_ADDR_32BIT     (0x08U)

#define OSPI_FIFO_SIZE      (128) // 256 words
#define OSPI_PAGE_SIZE      (128)
#define OSPI_SECTOR_SIZE    (4096)
#define OSPI_POLL_TIMEOUT   (1000000)

#define OSPI_WRSR_COMMAND   (0x01)
#define OSPI_RDSR_COMMAND   (0x05)
#define OSPI_RDSR_DCYCLES   (0)

#define OSPI_WRVL_COMMAND   (0x81)
#define OSPI_RDVL_COMMAND   (0x85)

#define OSPI_WRNV_COMMAND   (0xB1)
#define OSPI_RDNV_COMMAND   (0xB5)

#define OSPI_RDID_COMMAND   (0x9F)
#define OSPI_RDID_DCYCLES   (0)

#define OSPI_READ_COMMAND   (0x13)
#define OSPI_READ_DCYCLES   (0)

#define OSPI_WREN_COMMAND   (0x06)
#define OSPI_WRITE_COMMAND  (0x12)

#define OSPI_ERASE_COMMAND  (0x21)
#define OSPI_ERASE_CHIP_COMMAND (0xC7)

typedef struct ospi_flash {
    uint8_t ddr_mode;
    uint8_t spi_mode;
    ssi_regs_t *regs;
    aes_regs_t *aes_regs;
    volatile void *xip_base;
    uint8_t initialized;
    bool bswap16;
    bool recovered;
} ospi_flash_t;

static ospi_flash_t ospi = { 0 };

static void port_delay_ns(uint32_t ns) {
    for (size_t i = 0; i < (ns / 2.5); i++) {
    }
}

static void ospi_setup(ospi_flash_t *ospi, uint32_t addr_len, uint32_t ndf, uint32_t wait_cycles) {
    ospi_writel(ospi, ser, 0);
    spi_disable(ospi);

    uint32_t val = CTRLR0_IS_MST
                   | (OSPI_DFS_8BIT << CTRLR0_DFS_OFFSET)
                   | (OSPI_FRF_SINGLE << CTRLR0_SPI_FRF_OFFSET)
                   | ((ndf ? SPI_TMOD_TR : TMOD_TO) << CTRLR0_TMOD_OFFSET);

    ospi_writel(ospi, ctrlr0, val);
    ospi_writel(ospi, ctrlr1, (ndf ? (ndf - 1) : 0));

    val = TRANS_TYPE_FRF_DEFINED
          | (1 << CTRLR0_SPI_RXDS_EN_OFFSET)
          | (addr_len << CTRLR0_ADDR_L_OFFSET)
          | (OSPI_INST_8BIT << CTRLR0_INST_L_OFFSET)
          | (wait_cycles << CTRLR0_WAIT_CYCLES_OFFSET);

    ospi_writel(ospi, spi_ctrlr0, val);
    spi_enable(ospi);
}

static inline void ospi_push(ospi_flash_t *ospi, uint32_t data) {
    ospi_writel(ospi, data_reg, data);
}

static int ospi_flash_wait_busy(ospi_flash_t *ospi) {
    uint32_t timeout = OSPI_POLL_TIMEOUT;
    while ((ospi_readl(ospi, sr) & (SR_TF_EMPTY | SR_BUSY)) != SR_TF_EMPTY) {
        if (--timeout == 0) {
            return -1;
        }
    }
    return 0;
}

static int ospi_send(ospi_flash_t *ospi, uint32_t cmd, uint32_t addr_len,
                     uint32_t addr, uint32_t data_len, const uint8_t *data) {
    ospi_setup(ospi, addr_len, 0, 0);
    ospi_push(ospi, cmd);

    // Push the address (0, 24, or 32 bits).
    for (size_t i = 0; i < addr_len / 2; i++) {
        ospi_push(ospi, addr >> ((addr_len / 2 - 1) * 8 - i * 8));
    }

    for (size_t i = 0; i < data_len; i++) {
        ospi_push(ospi, data[i]);
    }

    ospi_writel(ospi, ser, OMV_BOOT_OSPI_SER);
    return ospi_flash_wait_busy(ospi);
}

static int ospi_recv(ospi_flash_t *ospi, uint32_t cmd, uint32_t addr_len,
                     uint32_t addr, size_t data_len, uint8_t *data, uint32_t dcycles) {

    ospi_setup(ospi, addr_len, data_len, dcycles);
    ospi_push(ospi, cmd);

    // Push the address (0, 24, or 32 bits).
    for (size_t i = 0; i < addr_len / 2; i++) {
        ospi_push(ospi, addr >> ((addr_len / 2 - 1) * 8 - i * 8));
    }

    // Push dummy bytes as many as the data size.
    for (size_t i = 0; i < data_len; i++) {
        ospi_push(ospi, 0x00);
    }

    ospi_writel(ospi, ser, OMV_BOOT_OSPI_SER);

    // Skip the command and address high-z bytes.
    uint32_t skip_len = addr_len / 2 + 1;

    for (size_t i = 0; i < skip_len + data_len; i++) {
        unsigned int timeout = 100000;

        while (ospi_readl(ospi, rxflr) == 0) {
            if (--timeout == 0) {
                return -1;
            }
        }

        if (i < skip_len) {
            (void) ospi_readl(ospi, data_reg);
        } else {
            data[i - skip_len] = ospi_readl(ospi, data_reg);
        }
    }
    return 0;
}

static int ospi_flash_wait_sr(ospi_flash_t *ospi, uint8_t mask, uint8_t val, uint32_t timeout) {
    do {
        uint8_t sr;
        if (ospi_recv(ospi, OSPI_RDSR_COMMAND, OSPI_ADDR_0BIT, 0, 1, &sr, OSPI_RDSR_DCYCLES) != 0) {
            return -1;
        }
        if ((sr & mask) == val) {
            return 0; // success
        }
    } while (timeout--);

    return -1;
}

static int ospi_flash_wait_wel1(ospi_flash_t *ospi) {
    return ospi_flash_wait_sr(ospi, 2, 2, OSPI_POLL_TIMEOUT);
}

static int ospi_flash_wait_wip0(ospi_flash_t *ospi) {
    return ospi_flash_wait_sr(ospi, 1, 0, OSPI_POLL_TIMEOUT);
}

static int ospi_flash_write_enable(ospi_flash_t *ospi) {
    ospi_send(ospi, OSPI_WREN_COMMAND, OSPI_ADDR_0BIT, 0, 0, NULL);
    return ospi_flash_wait_wel1(ospi);
}

int ospi_flash_read_reg(ospi_flash_t *ospi, uint32_t cmd, uint32_t addr_len, uint32_t addr) {
    uint8_t buf;
    if (ospi_recv(ospi, cmd, addr_len, addr, 1, &buf, 0) != 0) {
        return -1;
    }
    return buf;
}

int ospi_flash_write_reg(ospi_flash_t *ospi, uint32_t cmd, uint32_t addr_len, uint32_t addr, uint8_t buf) {
    if (ospi_flash_write_enable(ospi) != 0 ||
        ospi_send(ospi, cmd, addr_len, addr, 1, &buf) != 0 ||
        ospi_flash_wait_wip0(ospi) != 0) {
        return -1;
    }
    return 0;
}

static int ospi_flash_chip_erase(ospi_flash_t *ospi, uint32_t value) {
    // Configure erase value
    uint32_t nv_reg = 0x7F | ((value & 1) << 7);
    if (ospi_flash_write_reg(ospi, OSPI_WRVL_COMMAND, OSPI_ADDR_24BIT, 0x08, nv_reg) != 0) {
        return -1;
    }
    if (ospi_flash_write_enable(ospi) != 0 ||
        ospi_send(ospi, OSPI_ERASE_CHIP_COMMAND, OSPI_ADDR_0BIT, 0, 0, NULL) != 0 ||
        ospi_flash_wait_sr(ospi, 1, 0, 400000) != 0) {
        return -1;
    }
    return 0;
}

static int ospi_flash_reset(ospi_flash_t *ospi) {
    port_pin_write(OMV_BOOT_OSPI_RST_PIN, 0);
    port_delay_ms(1);
    port_pin_write(OMV_BOOT_OSPI_RST_PIN, 1);
    port_delay_ms(1);
    return 0;
}

static int ospi_flash_init(ospi_flash_t *ospi) {
    if (ospi->initialized) {
        return 0;
    }

    // Reset flash.
    ospi_flash_reset(ospi);

    // Use SPI mode for initialization.
    ospi->spi_mode = true;
    ospi->regs = (ssi_regs_t *) OMV_BOOT_OSPI_SSI_BASE;
    ospi->aes_regs = (aes_regs_t *) OMV_BOOT_OSPI_AES_BASE;
    ospi->xip_base = (volatile void *) OMV_BOOT_OSPI_XIP_BASE;

    // Initialize OSPI.
    spi_disable(ospi);
    ospi_writel(ospi, ser, 0);
    ospi_writel(ospi, rx_sample_dly, OMV_BOOT_OSPI_RX_DELAY);
    ospi_writel(ospi, txd_drive_edge, OMV_BOOT_OSPI_DDR_EDGE);
    ospi->aes_regs->aes_control &= ~AES_CONTROL_XIP_EN;
    ospi->aes_regs->aes_rxds_delay = OMV_BOOT_OSPI_RXDS_DELAY;
    spi_set_clk(ospi, (GetSystemAXIClock() / OMV_BOOT_OSPI_CLOCK));
    spi_enable(ospi);

    // Detect onboard flash based on ID.
    uint8_t id[3] = { 0 };
    ospi_recv(ospi, OSPI_RDID_COMMAND, OSPI_ADDR_0BIT, 0, 3, id, OSPI_RDID_DCYCLES);
    ospi->initialized = true;
    return 0;
}

static int ospi_flash_deinit(ospi_flash_t *ospi) {
    if (ospi->regs) {
        spi_disable(ospi);
        spi_set_clk(ospi, 0);
    }
    ospi->initialized = false;

    // Reset flash.
    return ospi_flash_reset(ospi);
}

static int ospi_flash_erase(ospi_flash_t *ospi, uint32_t addr) {
    if (ospi_flash_write_enable(ospi) != 0) {
        return -1;
    }
    if (ospi_send(ospi, OSPI_ERASE_COMMAND, OSPI_ADDR_32BIT, addr, 0, NULL) != 0) {
        return -1;
    }
    return ospi_flash_wait_wip0(ospi);
}

static int ospi_flash_read_page(ospi_flash_t *ospi, uint32_t addr, uint8_t *buf, uint32_t size) {
    return ospi_recv(ospi, OSPI_READ_COMMAND, OSPI_ADDR_32BIT, addr, size, buf, OSPI_READ_DCYCLES);
}

static int ospi_flash_write_page(ospi_flash_t *ospi, uint32_t addr, const uint8_t *buf, uint32_t size) {
    if (ospi_flash_write_enable(ospi) != 0) {
        return -1;
    }
    if (ospi_send(ospi, OSPI_WRITE_COMMAND, OSPI_ADDR_32BIT, addr, size, buf) != 0) {
        return -1;
    }
    return ospi_flash_wait_wip0(ospi);
}

int spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t size) {
    if (ospi_flash_init(&ospi) != 0) {
        return -1;
    }

    for (size_t i = 0; i < size / OSPI_FIFO_SIZE; i++) {
        if (ospi_flash_read_page(&ospi, addr, buf, OSPI_FIFO_SIZE) != 0) {
            return -1;
        }
        buf += OSPI_FIFO_SIZE;
        addr += OSPI_FIFO_SIZE;
    }

    if ((size % OSPI_FIFO_SIZE)) {
        return ospi_flash_read_page(&ospi, addr, buf, (size % OSPI_FIFO_SIZE));
    }
    return 0;
}

int spi_flash_write(uint32_t addr, const uint8_t *buf, uint32_t size) {
    if (ospi_flash_init(&ospi) != 0) {
        return -1;
    }

    if ((addr % OSPI_SECTOR_SIZE) == 0) {
        if (ospi_flash_erase(&ospi, addr) != 0) {
            return -1;
        }
    }

    if (ospi.bswap16) {
        uint16_t *buf16 = (uint16_t *) buf;
        for (size_t i = 0; i < size / 2; i++) {
            buf16[i] = __REV16(buf16[i]);
        }
    }

    for (size_t i = 0; i < size / OSPI_PAGE_SIZE; i++) {
        if (ospi_flash_write_page(&ospi, addr, buf, OSPI_PAGE_SIZE) != 0) {
            return -1;
        }
        buf += OSPI_PAGE_SIZE;
        addr += OSPI_PAGE_SIZE;
    }

    if ((size % OSPI_PAGE_SIZE)) {
        return ospi_flash_write_page(&ospi, addr, buf, (size % OSPI_PAGE_SIZE));
    }
    return 0;
}

int spi_flash_recovery(uint32_t addr, const uint8_t *buf, uint32_t size) {
    static const uint8_t key[] = {
        0x15, 0x9e, 0x7d, 0x42, 0x96, 0x1a, 0x71, 0xeb,
        0x73, 0xa3, 0x26, 0x29, 0x2b, 0x08, 0x09, 0x0e,
    };

    if (ospi.recovered) {
        return -1;
    }

    // Check recovery key.
    if (size < 16 || memcmp(buf, key, sizeof(key))) {
        return -1;
    }

    // Init flash.
    if (ospi_flash_init(&ospi) != 0) {
        return -1;
    }

    // 1. Perform JESD reset
    const pin_t *cs = &omv_boot_pins[OMV_BOOT_OSPI_CS_PIN];
    pinconf_set(cs->port, cs->pin, OMV_BOOT_ALT_GPIO, cs->pad);
    port_pin_mode(OMV_BOOT_OSPI_CS_PIN, PIN_MODE_OUTPUT);
    port_pin_write(OMV_BOOT_OSPI_CS_PIN, 1);

    const pin_t *d0 = &omv_boot_pins[OMV_BOOT_OSPI_D0_PIN];
    pinconf_set(d0->port, d0->pin, OMV_BOOT_ALT_GPIO, d0->pad);
    port_pin_mode(OMV_BOOT_OSPI_D0_PIN, PIN_MODE_OUTPUT);
    port_pin_write(OMV_BOOT_OSPI_D0_PIN, 0);

    // Clock out 0, 1, 0, 1
    for (size_t i = 0, v = 1; i < 4; v ^= 1, i++) {
        port_pin_write(OMV_BOOT_OSPI_CS_PIN, 0);
        port_delay_ns(500);

        port_pin_write(OMV_BOOT_OSPI_D0_PIN, v);

        port_pin_write(OMV_BOOT_OSPI_CS_PIN, 1);
        port_delay_ns(500);
    }

    // Revert pin state.
    pinconf_set(d0->port, d0->pin, d0->alt, d0->pad);
    pinconf_set(cs->port, cs->pin, cs->alt, cs->pad);

    // 2.Write Enable. Write Opcode 0x06 to set WEL bit to 1
    if (ospi_flash_write_enable(&ospi) != 0) {
        return -1;
    }

    // 4. DFIM entry
    if (ospi_flash_write_reg(&ospi, OSPI_WRVL_COMMAND, OSPI_ADDR_24BIT, 0x1E, 0x6B) != 0) {
        return -1;
    }

    if (ospi_flash_read_reg(&ospi, OSPI_RDVL_COMMAND, OSPI_ADDR_24BIT, 0x1E) != 1) {
        return -1;
    }

    for (size_t i = 0; i < 2; i++) {
        // 5.a Initialize status register to 0xFF
        if (ospi_flash_write_reg(&ospi, OSPI_WRSR_COMMAND, OSPI_ADDR_0BIT, 0x00, 0xFF) != 0) {
            return -1;
        }

        // 5.c Initialize status register to 0x00
        if (ospi_flash_write_reg(&ospi, OSPI_WRSR_COMMAND, OSPI_ADDR_0BIT, 0x00, 0x00) != 0) {
            return -1;
        }

        // 6.a Initialize NCRs (Command=0xB1; Addr=0x00 to 0x0C; Data=0x00)
        for (size_t addr = 0x00; addr <= 0x0C; addr++) {
            if (ospi_flash_write_reg(&ospi, 0xB1, OSPI_ADDR_24BIT, addr, 0x00) != 0) {
                return -1;
            }
        }

        // 6.c Initialize NCRs (Command=0xB1; Addr=0x00 to 0x0C; Data=0x00)
        for (size_t addr = 0x00; addr <= 0x0C; addr++) {
            if (ospi_flash_write_reg(&ospi, 0xB1, OSPI_ADDR_24BIT, addr, 0xFF) != 0) {
                return -1;
            }
        }
    }

    for (size_t i = 0; i < 2; i++) {
        // 7.a Disable lock: Command=0x81; Addr=0x08; Data=0xF9
        if (ospi_flash_write_reg(&ospi, 0x81, OSPI_ADDR_24BIT, 0x08, 0xF9) != 0) {
            return -1;
        }

        // 7.b Write 13: Command=0x42; Addr=0x00 to end (n=257); Data=0xFF
        for (size_t addr = 0; addr < 256; addr++) {
            if (ospi_flash_write_reg(&ospi, 0x42, OSPI_ADDR_24BIT, addr, 0xFF) != 0) {
                return -1;
            }
        }

        // 8.a Disable lock: Command=0x81; Addr=0x08; Data=0xF9
        if (ospi_flash_write_reg(&ospi, 0x81, OSPI_ADDR_24BIT, 0x08, 0xF9) != 0) {
            return -1;
        }

        // 8.b Write 03: Command=0x42; Addr=0x00 to end (n=257); Data=0x00
        for (size_t addr = 0; addr < 256; addr++) {
            if (ospi_flash_write_reg(&ospi, 0x42, OSPI_ADDR_24BIT, addr, 0x00) != 0) {
                return -1;
            }
        }

    }

    for (size_t i = 0; i < 2; i++) {
        // 9, 11 Write entire Device to 0
        if (ospi_flash_chip_erase(&ospi, 0) != 0) {
            return -1;
        }

        // 10, 12 Write entire Device to 1
        if (ospi_flash_chip_erase(&ospi, 1) != 0) {
            return -1;
        }
    }

    // DFIM exit
    if (ospi_flash_write_reg(&ospi, OSPI_WRVL_COMMAND, OSPI_ADDR_24BIT, 0x1E, 0) != 0) {
        return -1;
    }

    ospi.recovered = true;
    return 0;
}

int spi_flash_deinit() {
    return ospi_flash_deinit(&ospi);
}
#endif // OMV_BOOT_SPI_FLASH_ENABLE

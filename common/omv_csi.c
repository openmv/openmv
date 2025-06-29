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
 * CMOS sensor interface abstraction layer.
 * This file provides default functions that can be overriden by ports.
 */
#if MICROPY_PY_CSI
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "py/mphal.h"
#include "omv_gpio.h"
#include "omv_i2c.h"
#include "omv_csi.h"
#include "omv_boardconfig.h"
#include "framebuffer.h"
#include "unaligned_memcpy.h"
#include "sensor_config.h"

#ifndef OMV_CSI_RESET_DELAY
#define OMV_CSI_RESET_DELAY (10)
#endif

#ifndef OMV_CSI_POWER_DELAY
#define OMV_CSI_POWER_DELAY (10)
#endif

#ifndef OMV_CSI_I2C_MAX_DEV
#define OMV_CSI_I2C_MAX_DEV (10)
#endif

#ifndef OMV_CSI_I2C_REINIT
#define OMV_CSI_I2C_REINIT  (1)
#endif

#ifndef __weak
#define __weak    __attribute__((weak))
#endif

// Used for scanning the I2C bus.
typedef struct _i2c_dev {
    uint8_t slv_addr;   // I2C address.
    uint32_t chip_id;   // Chip ID.
} i2c_dev_t;

// Sensor frame size/resolution table.
uint16_t resolution[][2] = {
    {0,    0   },
    // C/SIF Resolutions
    {88,   72  },    /* QQCIF     */
    {176,  144 },    /* QCIF      */
    {352,  288 },    /* CIF       */
    {88,   60  },    /* QQSIF     */
    {176,  120 },    /* QSIF      */
    {352,  240 },    /* SIF       */
    // VGA Resolutions
    {40,   30  },    /* QQQQVGA   */
    {80,   60  },    /* QQQVGA    */
    {160,  120 },    /* QQVGA     */
    {320,  240 },    /* QVGA      */
    {640,  480 },    /* VGA       */
    {30,   20  },    /* HQQQQVGA  */
    {60,   40  },    /* HQQQVGA   */
    {120,  80  },    /* HQQVGA    */
    {240,  160 },    /* HQVGA     */
    {480,  320 },    /* HVGA      */
    // FFT Resolutions
    {64,   32  },    /* 64x32     */
    {64,   64  },    /* 64x64     */
    {128,  64  },    /* 128x64    */
    {128,  128 },    /* 128x128   */
    // Himax Resolutions
    {160,  160 },    /* 160x160   */
    {320,  320 },    /* 320x320   */
    // Other
    {128,  160 },    /* LCD       */
    {128,  160 },    /* QQVGA2    */
    {720,  480 },    /* WVGA      */
    {752,  480 },    /* WVGA2     */
    {800,  600 },    /* SVGA      */
    {1024, 768 },    /* XGA       */
    {1280, 768 },    /* WXGA      */
    {1280, 1024},    /* SXGA      */
    {1280, 960 },    /* SXGAM     */
    {1600, 1200},    /* UXGA      */
    {1280, 720 },    /* HD        */
    {1920, 1080},    /* FHD       */
    {2560, 1440},    /* QHD       */
    {2048, 1536},    /* QXGA      */
    {2560, 1600},    /* WQXGA     */
    {2592, 1944},    /* WQXGA2    */
};

omv_csi_t csi_all[OMV_CSI_MAX_DEVICES] = {0};

__weak void omv_csi_init0() {
    for (size_t i=0; i<OMV_CSI_MAX_DEVICES; i++) {
        omv_csi_t *csi = &csi_all[i];
        omv_i2c_t *i2c = csi->i2c;

        if (!csi->detected) {
            continue;
        }

        // Reset delays
        csi->disable_delays = false;

        omv_csi_set_vsync_callback(csi, (omv_csi_cb_t) { NULL, NULL });
        omv_csi_set_frame_callback(csi, (omv_csi_cb_t) { NULL, NULL });

        // Re-init i2c bus to reset the bus state after soft reset,
        // which could have interrupted the bus mid-transfer.
        if (i2c && i2c->initialized) {
            // Reinitialize the bus using the last used id and speed.
            #if OMV_CSI_I2C_REINIT
            // On some ports, this casues I2C/I3C to lock up.
            omv_i2c_init(i2c, i2c->id, i2c->speed);
            #endif
        }
    }
}

__weak int omv_csi_init() {
    // Reset the csi state
    memset(csi_all, 0, sizeof(csi_all));
    return OMV_CSI_ERROR_CTL_UNSUPPORTED;
}

omv_csi_t *omv_csi_get(int id) {
    omv_csi_t *csi = NULL;

    for (size_t i=0; !csi && i<OMV_CSI_MAX_DEVICES; i++) {
        if (id == -1 && !csi_all[i].auxiliary) {
            csi = &csi_all[i];
        } else if (omv_csi_match(&csi_all[i], id)) {
            csi = &csi_all[i];
        }
    }

    return csi;
}

__weak int omv_csi_match(omv_csi_t *csi, size_t id) {
    // Call the sensor specific function.
    if (csi->match != NULL) {
        return csi->match(csi, id);
    }

    return (csi->chip_id == id);
}

__weak int omv_csi_abort(omv_csi_t *csi, bool fifo_flush, bool in_irq) {
    // Call the sensor specific function.
    if (csi->abort != NULL &&
        csi->abort(csi, fifo_flush, in_irq) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    csi->first_line = false;
    csi->drop_frame = false;
    csi->last_frame_ms = 0;
    csi->last_frame_ms_valid = false;

    if (csi->fb) {
        if (fifo_flush) {
            framebuffer_flush_buffers(csi->fb, true);
        } else if (!csi->disable_full_flush) {
            framebuffer_flush_buffers(csi->fb, false);
        }
    }
    return 0;
}

void omv_csi_abort_all(void) {
    for (size_t i=0; i<OMV_CSI_MAX_DEVICES; i++) {
        omv_csi_t *csi = &csi_all[i];

        // Abort ongoing transfer
        if (csi->detected) {
            omv_csi_abort(csi, true, false);

        }
    }
}

__weak int omv_csi_reset(omv_csi_t *csi, bool hard) {
    // Disable any ongoing frame capture.
    if (csi->power_on) {
        omv_csi_abort(csi, true, false);
    }

    // Reset the csi state
    csi->sde = 0;
    csi->pixformat = 0;
    csi->framesize = 0;
    csi->framerate = 0;
    csi->first_line = false;
    csi->drop_frame = false;
    csi->last_frame_ms = 0;
    csi->last_frame_ms_valid = false;
    csi->gainceiling = 0;
    csi->hmirror = false;
    csi->vflip = false;
    csi->transpose = false;
    #if MICROPY_PY_IMU
    csi->auto_rotation = (csi->chip_id == OV7690_ID);
    #else
    csi->auto_rotation = false;
    #endif // MICROPY_PY_IMU
    csi->color_palette = rainbow_table;
    csi->disable_full_flush = false;
    csi->vsync_cb = (omv_csi_cb_t) { NULL, NULL };
    csi->frame_cb = (omv_csi_cb_t) { NULL, NULL };
   
    // Restore shutdown state on reset.
    if (!csi->power_on) {
        omv_csi_shutdown(csi, false);
    }

    if (hard) {
        // Disable the bus before reset.
        omv_i2c_enable(csi->i2c, false);

        #if defined(OMV_CSI_RESET_PIN)
        // Hard-reset the csi
        if (csi->reset_pol == OMV_CSI_ACTIVE_HIGH) {
            omv_gpio_write(OMV_CSI_RESET_PIN, 1);
            mp_hal_delay_ms(10);
            omv_gpio_write(OMV_CSI_RESET_PIN, 0);
        } else {
            omv_gpio_write(OMV_CSI_RESET_PIN, 0);
            mp_hal_delay_ms(10);
            omv_gpio_write(OMV_CSI_RESET_PIN, 1);
        }
        #endif

        mp_hal_delay_ms(OMV_CSI_RESET_DELAY);

        // Re-enable the bus.
        omv_i2c_enable(csi->i2c, true);
    }

    // Call csi-specific reset function
    if (csi->reset != NULL &&
        csi->reset(csi) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // Reset framebuffers
    framebuffer_flush_buffers(csi->fb, true);
    return 0;
}

static size_t omv_csi_detect(omv_i2c_t *i2c, i2c_dev_t *dev_list) {
    size_t dev_count = 0;
    uint8_t addr_list[OMV_CSI_I2C_MAX_DEV];
    int addr_count = omv_i2c_scan(i2c, addr_list, OMV_ARRAY_SIZE(addr_list));

    for (int i=0; i<addr_count; i++) {
        uint32_t chip_id = 0;
        uint8_t slv_addr = addr_list[i];

        switch (slv_addr) {
            #if (OMV_OV2640_ENABLE == 1)
            case OV2640_SLV_ADDR: // Or OV9650.
                omv_i2c_readb(i2c, slv_addr, OV_CHIP_ID, (uint8_t *) &chip_id);
                break;
            #endif // (OMV_OV2640_ENABLE == 1)

            #if (OMV_OV5640_ENABLE == 1) || (OMV_GC2145_ENABLE == 1) || (OMV_GENX320_ENABLE == 1)
            // OV5640, GC2145, and GENX320 share the same I2C address
            case OV5640_SLV_ADDR:   // Or GC2145, or GENX320.
                // Try to read GC2145 chip ID first
                omv_i2c_readb(i2c, slv_addr, GC_CHIP_ID, (uint8_t *) &chip_id);
                if (chip_id != GC2145_ID) {
                    // If it fails, try reading OV5640 chip ID.
                    omv_i2c_readb2(i2c, slv_addr, OV5640_CHIP_ID, (uint8_t *) &chip_id);

                    #if (OMV_GENX320_ENABLE == 1)
                    if (chip_id != OV5640_ID) {
                        // If it fails, try reading GENX320 chip ID.
                        uint8_t buf[] = {(GENX320_CHIP_ID >> 8), GENX320_CHIP_ID};
                        omv_i2c_write_bytes(i2c, slv_addr, buf, 2, OMV_I2C_XFER_NO_STOP);
                        omv_i2c_read_bytes(i2c, slv_addr,
                                           (uint8_t *) &chip_id, 4, OMV_I2C_XFER_NO_FLAGS);
                        chip_id = __REV(chip_id);
                    }
                    #endif // (OMV_GENX320_ENABLE == 1)
                }
                break;
            #endif // (OMV_OV5640_ENABLE == 1) || (OMV_GC2145_ENABLE == 1) || (OMV_GENX320_ENABLE == 1)

            #if (OMV_OV7725_ENABLE == 1) || (OMV_OV7670_ENABLE == 1) || (OMV_OV7690_ENABLE == 1)
            case OV7725_SLV_ADDR: // Or OV7690 or OV7670.
                omv_i2c_readb(i2c, slv_addr, OV_CHIP_ID, (uint8_t *) &chip_id);
                break;
            #endif //(OMV_OV7725_ENABLE == 1) || (OMV_OV7670_ENABLE == 1) || (OMV_OV7690_ENABLE == 1)

            #if (OMV_MT9V0XX_ENABLE == 1)
            case MT9V0XX_SLV_ADDR:
                omv_i2c_readw(i2c, slv_addr, ON_CHIP_ID, (uint16_t *) &chip_id);
                break;
            #endif //(OMV_MT9V0XX_ENABLE == 1)

            #if (OMV_BOSON_ENABLE == 1)
            case BOSON_SLV_ADDR:
                chip_id = BOSON_ID;
                break;
            #endif // (OMV_BOSON_ENABLE == 1)

            #if (OMV_LEPTON_ENABLE == 1)
            case LEPTON_SLV_ADDR:
                chip_id = LEPTON_ID;
                break;
            #endif // (OMV_LEPTON_ENABLE == 1)

            #if (OMV_HM01B0_ENABLE == 1) || (OMV_HM0360_ENABLE == 1)
            case HM0XX0_SLV_ADDR:
                omv_i2c_readb2(i2c, slv_addr, HIMAX_CHIP_ID, (uint8_t *) &chip_id);
                break;
            #endif // (OMV_HM01B0_ENABLE == 1) || (OMV_HM0360_ENABLE == 1)

            #if (OMV_FROGEYE2020_ENABLE == 1)
            case FROGEYE2020_SLV_ADDR:
                chip_id = FROGEYE2020_ID;
                break;
            #endif // (OMV_FROGEYE2020_ENABLE == 1)

            #if (OMV_PAG7920_ENABLE == 1) || (OMV_PAG7936_ENABLE == 1)
            // PAG720 and PAG7936 share the same I2C address.
            case PAG7920_SLV_ADDR:
            case PAG7936_SLV_ADDR_ALT:
                omv_i2c_readw2(i2c, slv_addr, PIXART_CHIP_ID, (uint16_t *) &chip_id);
                chip_id = ((chip_id << 8) | (chip_id >> 8)) & 0xFFFF;
                break;
            #endif // (OMV_PAG7920_ENABLE == 1) || (OMV_PAG7936_ENABLE == 1)

            #if (OMV_MT9M114_ENABLE == 1) || (OMV_PS5520_ENABLE == 1)
            // MT9M114 and PS5520 share the same I2C address.
            case MT9M114_SLV_ADDR:
                omv_i2c_readw2(i2c, slv_addr, ON_CHIP_ID, (uint16_t *) &chip_id);
                if (slv_addr != MT9M114_ID) {
                    omv_i2c_readw2(i2c, slv_addr, PIXART_CHIP_ID, (uint16_t *) &chip_id);
                }
                break;
            #endif // (OMV_MT9M114_ENABLE == 1) || (OMV_PS5520_ENABLE == 1)
        }

        if (chip_id && dev_count < OMV_CSI_MAX_DEVICES) {
            dev_list[dev_count++] = (i2c_dev_t) { slv_addr, chip_id };
        }
    }

    return dev_count;
}

int omv_csi_probe(omv_i2c_t *i2c) {
    bool reset_pol = OMV_CSI_ACTIVE_HIGH;
    bool power_pol = OMV_CSI_ACTIVE_HIGH;

    size_t dev_count = 0;
    size_t aux_count = 0;
    i2c_dev_t dev_list[OMV_CSI_MAX_DEVICES] = { 0 };

    // Active power-down state, active reset state
    // This order is required for all sensors to work correctly.
    const omv_csi_polarity_t polarity_configs[][2] = {
        { OMV_CSI_ACTIVE_HIGH, OMV_CSI_ACTIVE_HIGH },
        { OMV_CSI_ACTIVE_HIGH, OMV_CSI_ACTIVE_LOW },
        { OMV_CSI_ACTIVE_LOW,  OMV_CSI_ACTIVE_HIGH },
        { OMV_CSI_ACTIVE_LOW,  OMV_CSI_ACTIVE_LOW },
    };
    
    // Scan the bus multiple times using different reset and power polarities,
    // until a supported sensor is detected.
    for (size_t i=0; dev_count == 0 && i<OMV_ARRAY_SIZE(polarity_configs); i++) {
        // Power cycle
        #if defined(OMV_CSI_POWER_PIN)
        power_pol = polarity_configs[i][0];
        omv_gpio_write(OMV_CSI_POWER_PIN, power_pol);
        mp_hal_delay_ms(10);
        omv_gpio_write(OMV_CSI_POWER_PIN, !power_pol);
        mp_hal_delay_ms(OMV_CSI_POWER_DELAY);
        #endif

        // Reset
        #if defined(OMV_CSI_RESET_PIN)
        reset_pol = polarity_configs[i][1];
        omv_gpio_write(OMV_CSI_RESET_PIN, reset_pol);
        mp_hal_delay_ms(10);
        omv_gpio_write(OMV_CSI_RESET_PIN, !reset_pol);
        mp_hal_delay_ms(OMV_CSI_RESET_DELAY);
        #endif

        dev_count = omv_csi_detect(i2c, dev_list);
    }
    
    // Add special devices, such as SPI sensors, soft-CSI etc...
    #if OMV_SOFTCSI_ENABLE
    if (dev_count < OMV_CSI_MAX_DEVICES) {
        dev_list[dev_count++] = (i2c_dev_t) { 0, SOFTCSI_ID };
    }
    #endif

    #if OMV_PAJ6100_ENABLE
    if ((dev_count < OMV_CSI_MAX_DEVICES) && paj6100_detect(NULL)) {
        // Found PixArt PAJ6100
        power_pol = OMV_CSI_ACTIVE_LOW;
        reset_pol = OMV_CSI_ACTIVE_LOW;
        dev_list[dev_count++] = (i2c_dev_t) { 0, PAJ6100_ID };
    }
    #endif

    // Fail if no devices were detected and no special sensors enabled.
    if (!dev_count) {
        return OMV_CSI_ERROR_ISC_UNDETECTED;
    }

    // Initialize detected sensors.
    for (size_t i=0; i<dev_count; i++) {
        omv_csi_t *csi = &csi_all[i];

        csi->detected = true;
        csi->power_on = true;
        csi->power_pol = power_pol;
        csi->reset_pol = reset_pol;
        csi->chip_id =  dev_list[i].chip_id;
        csi->slv_addr = dev_list[i].slv_addr;

        uint32_t clk_hz = 0;
        sensor_init_t init_fun = NULL;

        // Find the sensors init function.
        for (size_t i=0; i<OMV_ARRAY_SIZE(sensor_config_table); i++) {
            const sensor_config_t *config = &sensor_config_table[i];
            if (csi->chip_id == config->chip_id) {
                clk_hz = config->clk_hz;
                init_fun = config->init_fun;
                break;
            }
        }

        if (init_fun ==  NULL) {
            return OMV_CSI_ERROR_ISC_UNSUPPORTED;
        } else if (init_fun(csi) != 0) {
            return OMV_CSI_ERROR_ISC_INIT_FAILED;
        }

        // Special case for OV5640.
        #if (OMV_OV5640_REV_Y_CHECK == 1)
        if (csi->chip_id == OV5640_ID && HAL_GetREVID() < 0x2003) {
            clk_hz = OMV_OV5640_REV_Y_FREQ;
        }
        #endif

        // Allow reconfiguring (or disabling) the external clock
        // if just one sensor is detected, or for main sensors.
        if (dev_count == 1 || !csi->auxiliary) {
            omv_csi_set_clk_frequency(clk_hz);
        }

        // Count aux devices.
        aux_count += csi->auxiliary;
    }

    // Special case: A single aux sensor was detected, clear
    // the auxiliary flag so it gets used as the main sensor.
    if (dev_count == 1 && csi_all[0].auxiliary) {
        csi_all[0].auxiliary = 0;
        aux_count--;
    }

    // Special case: Soft-CSI and another aux sensor detected,
    // (Lepton for example). Use Soft-CSI for the main sensor.
    if (dev_count == aux_count) {
        for (size_t i=0; i<dev_count; i++) {
            omv_csi_t *csi = &csi_all[i];
            if (csi->chip_id == SOFTCSI_ID) {
                csi->auxiliary = 0;
                aux_count--;
                break;
            }
        }
    }

    // There should be exactly 1 main sensor left. Any other
    // configuration is not supported.
    if ((dev_count - aux_count) != 1) {
        return -1;
    }
    
    // Clear the FB pointer for all aux sensors, as they use
    // dynamically allocated frame buffers.
    for (size_t i=0; i<dev_count; i++) {
        omv_csi_t *csi = &csi_all[i];
        if (csi->auxiliary) {
            csi->fb = NULL;
        }
    }

    return 0;
}

__weak int omv_csi_config(omv_csi_t *csi, omv_csi_config_t config) {
    // Call the sensor specific function.
    if (csi->detected &&
        csi->config != NULL &&
        csi->config(csi, config) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }
    return 0;
}

__weak int omv_csi_get_id(omv_csi_t *csi) {
    return csi->chip_id;
}

__weak uint32_t omv_csi_get_clk_frequency() {
    return OMV_CSI_ERROR_CTL_UNSUPPORTED;
}

__weak int omv_csi_set_clk_frequency(uint32_t frequency) {
    return OMV_CSI_ERROR_CTL_UNSUPPORTED;
}

__weak bool omv_csi_is_detected(omv_csi_t *csi) {
    return csi->detected;
}

__weak int omv_csi_sleep(omv_csi_t *csi, int enable) {
    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Call the sensor specific function.
    if (csi->sleep != NULL &&
        csi->sleep(csi, enable) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_shutdown(omv_csi_t *csi, int enable) {
    int ret = 0;

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    #if defined(OMV_CSI_POWER_PIN)
    if (enable) {
        if (csi->power_pol == OMV_CSI_ACTIVE_HIGH) {
            omv_gpio_write(OMV_CSI_POWER_PIN, 1);
        } else {
            omv_gpio_write(OMV_CSI_POWER_PIN, 0);
        }
    } else {
        if (csi->power_pol == OMV_CSI_ACTIVE_HIGH) {
            omv_gpio_write(OMV_CSI_POWER_PIN, 0);
        } else {
            omv_gpio_write(OMV_CSI_POWER_PIN, 1);
        }
        mp_hal_delay_ms(OMV_CSI_POWER_DELAY);
    }
    #endif

    // Call csi-specific shutdown function
    if (csi->shutdown != NULL &&
        csi->shutdown(csi, enable) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // Update power-on flag.
    csi->power_on = !enable;

    return ret;
}

__weak int omv_csi_read_reg(omv_csi_t *csi, uint16_t reg_addr) {
    int ret;

    // Check if the control is supported.
    if (csi->read_reg == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if ((ret = csi->read_reg(csi, reg_addr)) == -1) {
        return OMV_CSI_ERROR_IO_ERROR;
    }

    return ret;
}

__weak int omv_csi_write_reg(omv_csi_t *csi, uint16_t reg_addr, uint16_t reg_data) {
    // Check if the control is supported.
    if (csi->write_reg == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->write_reg(csi, reg_addr, reg_data) == -1) {
        return OMV_CSI_ERROR_IO_ERROR;
    }

    return 0;
}

__weak int omv_csi_set_pixformat(omv_csi_t *csi, pixformat_t pixformat) {
    // Check if the value has changed.
    if (csi->pixformat == pixformat) {
        return 0;
    }

    // Some sensor drivers automatically switch to BAYER to reduce the frame size if it does not fit in RAM.
    // If the current format is BAYER (1BPP), and the target format is color and (2BPP), and the frame does not
    // fit in RAM it will just be switched back again to BAYER, so we keep the current format unchanged.
    uint32_t size = framebuffer_get_buffer_size(csi->fb);
    if ((csi->pixformat == PIXFORMAT_BAYER) &&
        ((pixformat == PIXFORMAT_RGB565) || (pixformat == PIXFORMAT_YUV422)) &&
        (csi->fb->u * csi->fb->v * 2 > size) &&
        (csi->fb->u * csi->fb->v * 1 <= size)) {
        return 0;
    }

    // Cropping and transposing (and thus auto rotation) don't work in JPEG mode.
    if (((pixformat == PIXFORMAT_YUV422) && (csi->transpose || csi->auto_rotation)) ||
        ((pixformat == PIXFORMAT_JPEG) && (omv_csi_get_cropped(csi) || csi->transpose || csi->auto_rotation))) {
        return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Check if the control is supported.
    if (csi->set_pixformat == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_pixformat(csi, pixformat) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    if (!csi->disable_delays) {
        mp_hal_delay_ms(100); // wait for the camera to settle
    }

    // Set pixel format
    csi->pixformat = pixformat;

    // Reset pixel format to skip the first frame.
    csi->fb->pixfmt = PIXFORMAT_INVALID;

    // Auto-adjust the number of frame buffers.
    omv_csi_set_framebuffers(csi, -1);

    // Reconfigure the hardware if needed.
    return omv_csi_config(csi, OMV_CSI_CONFIG_PIXFORMAT);
}

__weak int omv_csi_set_framesize(omv_csi_t *csi, omv_csi_framesize_t framesize) {
    if (csi->framesize == framesize) {
        // No change
        return 0;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Call the sensor specific function
    if (csi->set_framesize == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    if (csi->set_framesize(csi, framesize) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    if (!csi->disable_delays) {
        mp_hal_delay_ms(100); // wait for the camera to settle
    }

    // Set framebuffer size
    csi->framesize = framesize;

    // Set x and y offsets.
    csi->fb->x = 0;
    csi->fb->y = 0;
    // Set width and height.
    csi->fb->w = resolution[framesize][0];
    csi->fb->h = resolution[framesize][1];
    // Set backup width and height.
    csi->fb->u = resolution[framesize][0];
    csi->fb->v = resolution[framesize][1];
    // Reset pixel format to skip the first frame.
    csi->fb->pixfmt = PIXFORMAT_INVALID;

    // Auto-adjust the number of frame buffers.
    omv_csi_set_framebuffers(csi, -1);

    // Reconfigure the hardware if needed.
    return omv_csi_config(csi, OMV_CSI_CONFIG_FRAMESIZE);
}

__weak int omv_csi_set_framerate(omv_csi_t *csi, int framerate) {
    if (csi->framerate == framerate) {
        // No change
        return 0;
    }

    if (framerate < 0) {
        return OMV_CSI_ERROR_INVALID_ARGUMENT;
    }

    // If the csi implements framerate control use it.
    if (csi->set_framerate != NULL &&
        csi->set_framerate(csi, framerate) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    } else {
        // Otherwise use software framerate control.
        csi->framerate = framerate;
    }
    return 0;
}

__weak void omv_csi_throttle_framerate(omv_csi_t *csi) {
    if (!csi->first_line) {
        csi->first_line = true;
        uint32_t tick = mp_hal_ticks_ms();
        uint32_t framerate_ms = IM_DIV(1000, csi->framerate);

        if (csi->last_frame_ms_valid && ((tick - csi->last_frame_ms) < framerate_ms)) {
            // Drop the current frame to match the requested frame rate. Note that if the frame
            // is marked to be dropped, it should not be copied to SRAM/SDRAM to save CPU time.
            csi->drop_frame = true;
        } else if (csi->last_frame_ms_valid) {
            csi->last_frame_ms += framerate_ms;
        } else {
            csi->last_frame_ms = tick;
            csi->last_frame_ms_valid = true;
        }
    }
}

__weak bool omv_csi_get_cropped(omv_csi_t *csi) {
    if (csi->framesize != OMV_CSI_FRAMESIZE_INVALID) {
        return (csi->fb->x != 0) ||
               (csi->fb->y != 0) ||
               (csi->fb->u != resolution[csi->framesize][0]) ||
               (csi->fb->v != resolution[csi->framesize][1]);
    }
    return false;
}

__weak uint32_t omv_csi_get_src_bpp(omv_csi_t *csi) {
    if (csi->raw_output) {
        return 1;
    }
    switch (csi->pixformat) {
        case PIXFORMAT_BAYER:
        case PIXFORMAT_JPEG:
            return 1;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        case PIXFORMAT_GRAYSCALE:
            return csi->mono_bpp;
        default:
            return 0;
    }
}

__weak uint32_t omv_csi_get_dst_bpp(omv_csi_t *csi) {
    switch (csi->pixformat) {
        case PIXFORMAT_GRAYSCALE:
        case PIXFORMAT_BAYER:
            return 1;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            return 2;
        default:
            return 0;
    }
}

__weak int omv_csi_set_windowing(omv_csi_t *csi, int x, int y, int w, int h) {
    // Check if the value has changed.
    if ((csi->fb->x == x) && (csi->fb->y == y) &&
        (csi->fb->u == w) && (csi->fb->v == h)) {
        return 0;
    }

    if (csi->pixformat == PIXFORMAT_JPEG) {
        return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Set x and y offsets.
    csi->fb->x = x;
    csi->fb->y = y;
    // Set width and height.
    csi->fb->w = w;
    csi->fb->h = h;
    // Set backup width and height.
    csi->fb->u = w;
    csi->fb->v = h;
    // Reset pixel format to skip the first frame.
    csi->fb->pixfmt = PIXFORMAT_INVALID;

    // Auto-adjust the number of frame buffers.
    omv_csi_set_framebuffers(csi, -1);

    // Reconfigure the hardware if needed.
    return omv_csi_config(csi, OMV_CSI_CONFIG_WINDOWING);
}

__weak int omv_csi_set_contrast(omv_csi_t *csi, int level) {
    // Check if the control is supported.
    if (csi->set_contrast == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_contrast(csi, level) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_brightness(omv_csi_t *csi, int level) {
    // Check if the control is supported.
    if (csi->set_brightness == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_brightness(csi, level) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_saturation(omv_csi_t *csi, int level) {
    // Check if the control is supported.
    if (csi->set_saturation == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_saturation(csi, level) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_gainceiling(omv_csi_t *csi, omv_csi_gainceiling_t gainceiling) {
    // Check if the value has changed.
    if (csi->gainceiling == gainceiling) {
        return 0;
    }

    // Check if the control is supported.
    if (csi->set_gainceiling == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_gainceiling(csi, gainceiling) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    csi->gainceiling = gainceiling;

    return 0;
}

__weak int omv_csi_set_quality(omv_csi_t *csi, int qs) {
    // Check if the control is supported.
    if (csi->set_quality == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_quality(csi, qs) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_colorbar(omv_csi_t *csi, int enable) {
    // Check if the control is supported.
    if (csi->set_colorbar == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_colorbar(csi, enable) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_auto_gain(omv_csi_t *csi, int enable, float gain_db, float gain_db_ceiling) {
    // Check if the control is supported.
    if (csi->set_auto_gain == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_auto_gain(csi, enable, gain_db, gain_db_ceiling) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_get_gain_db(omv_csi_t *csi, float *gain_db) {
    // Check if the control is supported.
    if (csi->get_gain_db == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->get_gain_db(csi, gain_db) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_auto_exposure(omv_csi_t *csi, int enable, int exposure_us) {
    // Check if the control is supported.
    if (csi->set_auto_exposure == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_auto_exposure(csi, enable, exposure_us) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_get_exposure_us(omv_csi_t *csi, int *exposure_us) {
    // Check if the control is supported.
    if (csi->get_exposure_us == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->get_exposure_us(csi, exposure_us) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_auto_whitebal(omv_csi_t *csi, int enable, float r_gain_db, float g_gain_db, float b_gain_db) {
    // Check if the control is supported.
    if (csi->set_auto_whitebal == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_auto_whitebal(csi, enable, r_gain_db, g_gain_db, b_gain_db) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_get_rgb_gain_db(omv_csi_t *csi, float *r_gain_db, float *g_gain_db, float *b_gain_db) {
    // Check if the control is supported.
    if (csi->get_rgb_gain_db == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->get_rgb_gain_db(csi, r_gain_db, g_gain_db, b_gain_db) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_auto_blc(omv_csi_t *csi, int enable, int *regs) {
    // Check if the control is supported.
    if (csi->set_auto_blc == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_auto_blc(csi, enable, regs) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_get_blc_regs(omv_csi_t *csi, int *regs) {
    // Check if the control is supported.
    if (csi->get_blc_regs == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->get_blc_regs(csi, regs) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_set_hmirror(omv_csi_t *csi, int enable) {
    // Check if the value has changed.
    if (csi->hmirror == ((bool) enable)) {
        return 0;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Check if the control is supported.
    if (csi->set_hmirror == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_hmirror(csi, enable) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    csi->hmirror = enable;

    // Wait for the camera to settle
    if (!csi->disable_delays) {
        mp_hal_delay_ms(100);
    }

    return 0;
}

__weak bool omv_csi_get_hmirror(omv_csi_t *csi) {
    return csi->hmirror;
}

__weak int omv_csi_set_vflip(omv_csi_t *csi, int enable) {
    // Check if the value has changed.
    if (csi->vflip == ((bool) enable)) {
        return 0;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Check if the control is supported.
    if (csi->set_vflip == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_vflip(csi, enable) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    csi->vflip = enable;

    // Wait for the camera to settle
    if (!csi->disable_delays) {
        mp_hal_delay_ms(100);
    }

    return 0;
}

__weak bool omv_csi_get_vflip(omv_csi_t *csi) {
    return csi->vflip;
}

__weak int omv_csi_set_transpose(omv_csi_t *csi, bool enable) {
    // Check if the value has changed.
    if (csi->transpose == enable) {
        return 0;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    if ((csi->pixformat == PIXFORMAT_YUV422) || (csi->pixformat == PIXFORMAT_JPEG)) {
        return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Set the new control value.
    csi->transpose = enable;

    return 0;
}

__weak bool omv_csi_get_transpose(omv_csi_t *csi) {
    return csi->transpose;
}

__weak int omv_csi_set_auto_rotation(omv_csi_t *csi, bool enable) {
    // Check if the value has changed.
    if (csi->auto_rotation == enable) {
        return 0;
    }

    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    // Operation not supported on JPEG images.
    if ((csi->pixformat == PIXFORMAT_YUV422) || (csi->pixformat == PIXFORMAT_JPEG)) {
        return OMV_CSI_ERROR_PIXFORMAT_UNSUPPORTED;
    }

    // Set the new control value.
    csi->auto_rotation = enable;
    return 0;
}

__weak bool omv_csi_get_auto_rotation(omv_csi_t *csi) {
    return csi->auto_rotation;
}

__weak int omv_csi_set_framebuffers(omv_csi_t *csi, int count) {
    // Disable any ongoing frame capture.
    omv_csi_abort(csi, true, false);

    if (csi->pixformat == PIXFORMAT_INVALID) {
        return OMV_CSI_ERROR_INVALID_PIXFORMAT;
    }

    if (csi->framesize == OMV_CSI_FRAMESIZE_INVALID) {
        return OMV_CSI_ERROR_INVALID_FRAMESIZE;
    }

    #if OMV_CSI_HW_CROP_ENABLE
    // If hardware cropping is supported, use window size.
    csi->fb->frame_size = csi->fb->u * csi->fb->v * 2;
    #else
    // Otherwise, use the real frame size.
    csi->fb->frame_size = resolution[csi->framesize][0] * resolution[csi->framesize][1] * 2;
    #endif
    return framebuffer_set_buffers(csi->fb, count);
}

__weak int omv_csi_set_special_effect(omv_csi_t *csi, omv_csi_sde_t sde) {
    // Check if the value has changed.
    if (csi->sde == sde) {
        return 0;
    }

    // Check if the control is supported.
    if (csi->set_special_effect == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_special_effect(csi, sde) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    // Set the new control value.
    csi->sde = sde;

    return 0;
}

__weak int omv_csi_set_lens_correction(omv_csi_t *csi, int enable, int radi, int coef) {
    // Check if the control is supported.
    if (csi->set_lens_correction == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    // Call the sensor specific function.
    if (csi->set_lens_correction(csi, enable, radi, coef) != 0) {
        return OMV_CSI_ERROR_CTL_FAILED;
    }

    return 0;
}

__weak int omv_csi_ioctl(omv_csi_t *csi, int request, ... /* arg */) {
    // Disable any ongoing frame capture.
    if (request & OMV_CSI_IOCTL_FLAGS_ABORT) {
        omv_csi_abort(csi, true, false);
    }

    // Check if the control is supported.
    if (csi->ioctl == NULL) {
        return OMV_CSI_ERROR_CTL_UNSUPPORTED;
    }

    va_list ap;
    va_start(ap, request);
    // Call the sensor specific function.
    int ret = csi->ioctl(csi, request, ap);
    va_end(ap);

    return ((ret != 0) ? OMV_CSI_ERROR_CTL_FAILED : 0);
}

__weak int omv_csi_set_vsync_callback(omv_csi_t *csi, omv_csi_cb_t cb) {
    csi->vsync_cb = cb;
    return 0;
}

__weak int omv_csi_set_frame_callback(omv_csi_t *csi, omv_csi_cb_t cb) {
    csi->frame_cb = cb;
    return 0;
}

__weak int omv_csi_set_color_palette(omv_csi_t *csi, const uint16_t *color_palette) {
    csi->color_palette = color_palette;
    return 0;
}

__weak const uint16_t *omv_csi_get_color_palette(omv_csi_t *csi) {
    return csi->color_palette;
}

__weak int omv_csi_check_framebuffer_size(omv_csi_t *csi) {
    uint32_t bpp = omv_csi_get_dst_bpp(csi);
    uint32_t size = framebuffer_get_buffer_size(csi->fb);
    return (((csi->fb->u * csi->fb->v * bpp) <= size) ? 0 : -1);
}

__weak int omv_csi_auto_crop_framebuffer(omv_csi_t *csi) {
    uint32_t bpp = omv_csi_get_dst_bpp(csi);
    uint32_t size = framebuffer_get_buffer_size(csi->fb);

    // If the pixformat is NULL/JPEG there we can't do anything to check if it fits before hand.
    if (!bpp) {
        return 0;
    }

    // csi->fb fits, we are done.
    if ((csi->fb->u * csi->fb->v * bpp) <= size) {
        return 0;
    }

    if ((csi->pixformat == PIXFORMAT_RGB565) || (csi->pixformat == PIXFORMAT_YUV422)) {
        // Switch to bayer for the quick 2x savings.
        omv_csi_set_pixformat(csi, PIXFORMAT_BAYER);
        bpp = 1;

        // csi->fb fits, we are done (bpp is 1).
        if ((csi->fb->u * csi->fb->v) <= size) {
            return 0;
        }
    }

    int window_w = csi->fb->u;
    int window_h = csi->fb->v;

    // We need to shrink the frame buffer. We can do this by cropping. So, we will subtract columns
    // and rows from the frame buffer until it fits within the frame buffer.
    int max = IM_MAX(window_w, window_h);
    int min = IM_MIN(window_w, window_h);
    float aspect_ratio = max / ((float) min);
    float r = aspect_ratio, best_r = r;
    int c = 1, best_c = c;
    float best_err = FLT_MAX;

    // Find the width/height ratio that's within 1% of the aspect ratio with a loop limit.
    for (int i = 100; i; i--) {
        float err = fast_fabsf(r - fast_roundf(r));

        if (err <= best_err) {
            best_err = err;
            best_r = r;
            best_c = c;
        }

        if (best_err <= 0.01f) {
            break;
        }

        r += aspect_ratio;
        c += 1;
    }

    // Select the larger geometry to map the aspect ratio to.
    int u_sub, v_sub;

    if (window_w > window_h) {
        u_sub = fast_roundf(best_r);
        v_sub = best_c;
    } else {
        u_sub = best_c;
        v_sub = fast_roundf(best_r);
    }

    // Crop the frame buffer while keeping the aspect ratio and keeping the width/height even.
    while (((csi->fb->u * csi->fb->v * bpp) > size) || (csi->fb->u % 2) || (csi->fb->v % 2)) {
        csi->fb->u -= u_sub;
        csi->fb->v -= v_sub;
    }

    // Center the new window using the previous offset and keep the offset even.
    csi->fb->x += (window_w - csi->fb->u) / 2;
    csi->fb->y += (window_h - csi->fb->v) / 2;

    if (csi->fb->x % 2) {
        csi->fb->x -= 1;
    }
    if (csi->fb->y % 2) {
        csi->fb->y -= 1;
    }

    // Auto-adjust the number of frame buffers.
    omv_csi_set_framebuffers(csi, -1);
    return 0;
}

#define copy_transposed_line(dstp, srcp)                   \
    for (int i = csi->fb->u, h = csi->fb->v; i; i--) { \
        *dstp = *srcp++;                                   \
        dstp += h;                                         \
    }

#define copy_transposed_line_rev16(dstp, srcp)             \
    for (int i = csi->fb->u, h = csi->fb->v; i; i--) { \
        *dstp = __REV16(*srcp++);                          \
        dstp += h;                                         \
    }

__weak int omv_csi_copy_line(omv_csi_t *csi, void *dma, uint8_t *src, uint8_t *dst) {
    uint16_t *src16 = (uint16_t *) src;
    uint16_t *dst16 = (uint16_t *) dst;
    #if OMV_CSI_DMA_MEMCPY_ENABLE
    extern int omv_csi_dma_memcpy(omv_csi_t *csi, void *dma, void *dst, void *src, int bpp, bool transposed);
    #endif

    switch (csi->pixformat) {
        case PIXFORMAT_BAYER:
            #if OMV_CSI_DMA_MEMCPY_ENABLE
            if (!omv_csi_dma_memcpy(csi, dma, dst, src, sizeof(uint8_t), csi->transpose)) {
                break;
            }
            #endif
            if (!csi->transpose) {
                unaligned_memcpy(dst, src, csi->fb->u);
            } else {
                copy_transposed_line(dst, src);
            }
            break;
        case PIXFORMAT_GRAYSCALE:
            #if OMV_CSI_DMA_MEMCPY_ENABLE
            if (!omv_csi_dma_memcpy(csi, dma, dst, src, sizeof(uint8_t), csi->transpose)) {
                break;
            }
            #endif
            if (csi->mono_bpp == 1) {
                // 1BPP GRAYSCALE.
                if (!csi->transpose) {
                    unaligned_memcpy(dst, src, csi->fb->u);
                } else {
                    copy_transposed_line(dst, src);
                }
            } else {
                // Extract Y channel from YUV.
                if (!csi->transpose) {
                    unaligned_2_to_1_memcpy(dst, src16, csi->fb->u);
                } else {
                    copy_transposed_line(dst, src16);
                }
            }
            break;
        case PIXFORMAT_RGB565:
        case PIXFORMAT_YUV422:
            #if OMV_CSI_DMA_MEMCPY_ENABLE
            if (!omv_csi_dma_memcpy(csi, dma, dst16, src16, sizeof(uint16_t), csi->transpose)) {
                break;
            }
            #endif
            if (0) {
            #if !OMV_CSI_HW_SWAP_ENABLE
            } else if ((csi->pixformat == PIXFORMAT_RGB565 && csi->rgb_swap) ||
                       (csi->pixformat == PIXFORMAT_YUV422 && csi->yuv_swap)) {
                if (!csi->transpose) {
                    unaligned_memcpy_rev16(dst16, src16, csi->fb->u);
                } else {
                    copy_transposed_line_rev16(dst16, src16);
                }
            #endif
            } else {
                if (!csi->transpose) {
                    unaligned_memcpy(dst16, src16, csi->fb->u * sizeof(uint16_t));
                } else {
                    copy_transposed_line(dst16, src16);
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

__weak int omv_csi_snapshot(omv_csi_t *csi, image_t *image, uint32_t flags) {
    int ret = OMV_CSI_ERROR_CTL_UNSUPPORTED;

    // Call the sensor specific function.
    if (csi->snapshot) {
        ret = csi->snapshot(csi, image, flags);
    }

    return ret;
}

const char *omv_csi_strerror(int error) {
    static const char *omv_csi_errors[] = {
        "No error.",
        "Sensor control failed.",
        "The requested operation is not supported by the image sensor.",
        "Failed to detect the image sensor or image sensor is detached.",
        "The detected image sensor is not supported.",
        "Failed to initialize the image sensor.",
        "Failed to initialize the external clock.",
        "Failed to initialize the CSI DMA.",
        "Failed to initialize the CSI interface.",
        "An low level I/O error has occurred.",
        "Frame capture has failed.",
        "Frame capture has timed out.",
        "Frame size is not supported or is not set.",
        "Pixel format is not supported or is not set.",
        "Window is not supported or is not set.",
        "Frame rate is not supported or is not set.",
        "An invalid argument is used.",
        "The requested operation is not supported on the current pixel format.",
        "Frame buffer error.",
        "Frame buffer overflow, try reducing the frame size.",
        "JPEG frame buffer overflow.",
        "The requested operation would block.",
    };

    // Sensor errors are negative.
    error = ((error < 0) ? (error * -1) : error);

    if (error > (sizeof(omv_csi_errors) / sizeof(omv_csi_errors[0]))) {
        return "Unknown error.";
    } else {
        return omv_csi_errors[error];
    }
}
#endif //MICROPY_PY_CSI

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
 * USB descriptors.
 */
#include <stdint.h>
#include <stdbool.h>
#include "omv_bootconfig.h"

#define STR_DESC_MAX_LEN        (32)
#define STR_DESC_COUNT          (sizeof(desc_string) / sizeof(desc_string[0]))

#define DFU_DESC_ALT_COUNT      OMV_BOOT_PARTITIONS_COUNT
#define DFU_DESC_FUNC_ATTR      (DFU_ATTR_CAN_UPLOAD | DFU_ATTR_CAN_DOWNLOAD | DFU_ATTR_MANIFESTATION_TOLERANT)

#define CFG_DESC_ITF_DFU        (0)
#define CFG_DESC_ITF_COUNT      (1)
#define CFG_DESC_TOTAL_SIZE     (TUD_CONFIG_DESC_LEN + TUD_DFU_DESC_LEN(DFU_DESC_ALT_COUNT))

static char const *desc_string[] = {
    "\x09\x04",                 // 0: English (0x0409)
    "OpenMV",                   // 1: Manufacturer
    "OpenMV Camera (DFU Mode)", // 2: Product
    OMV_BOOT_PARTITIONS_STR,
};

static tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = OMV_BOOT_VID,
    .idProduct = OMV_BOOT_PID,
    .bcdDevice = 0x0200,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

#if (CFG_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED)
// Device qualifier descriptor for high-speed devices.
const tusb_desc_device_qualifier_t desc_device_qualifier = {
    .bLength = sizeof(tusb_desc_device_qualifier_t),
    .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .bNumConfigurations = 0x01,
    .bReserved = 0x00,
};
#endif

static uint8_t const desc_config[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, CFG_DESC_ITF_COUNT, 0, CFG_DESC_TOTAL_SIZE, 0x00, 500),

    // Interface number, Alternate count, starting string index, attributes, detach timeout, transfer size
    TUD_DFU_DESCRIPTOR(CFG_DESC_ITF_DFU, DFU_DESC_ALT_COUNT, 4, DFU_DESC_FUNC_ATTR, 1000, CFG_TUD_DFU_XFER_BUFSIZE),
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

#if (CFG_TUD_MAX_SPEED == OPT_MODE_HIGH_SPEED)
uint8_t const *tud_descriptor_device_qualifier_cb(void) {
    return (uint8_t const *) &desc_device_qualifier;
}
#endif

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    return desc_config;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    size_t desc_len = 0;
    static uint16_t desc_str[STR_DESC_MAX_LEN + 1]; // + 1 for string type

    uint8_t uid[12] = {0};
    char hex_buf[sizeof(uid) * 2] = {0};
    const char hex_chars[] = "0123456789ABCDEF";
    const char *str = NULL;

    if (index == 0) {
        // Language ID
        desc_len = 1;
        memcpy(&desc_str[1], desc_string[0], 2);
    } else if (index == 3) {
        // Serial number
        if (port_get_uid(uid) != 0) {
            return NULL;
        }
        for (size_t i = 0; i < sizeof(uid); i++) {
            hex_buf[i * 2] = hex_chars[(uid[i] & 0xF0) >> 4];
            hex_buf[i * 2 + 1] = hex_chars[uid[i] & 0x0F];
        }
        str = hex_buf;
        desc_len = sizeof(uid) * 2;
    } else if (index < STR_DESC_COUNT) {
        // Other.
        str = desc_string[index];
        desc_len = strlen(str);
    } else {
        return NULL;
    }

    // Check max string descriptor length.
    if (desc_len > STR_DESC_MAX_LEN) {
        desc_len = STR_DESC_MAX_LEN;
    }

    // Convert to UTF-16
    for (size_t i = 0; i < desc_len; i++) {
        desc_str[1 + i] = str[i];
    }

    // First byte is the length (including the header), the second byte is string type.
    desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * desc_len + 2));
    return desc_str;
}

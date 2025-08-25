/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2025 OpenMV, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * OpenMV Protocol - Transport-agnostic communication protocol
 * This protocol provides reliable communication between host and
 * device with support for multiple transport layers (USB, UART, TCP/IP).
 */
#include <string.h>
#include <stdlib.h>

#include "py/runtime.h"
#include "shared/runtime/pyexec.h"
#include "boot/include/version.h"

#include "omv_common.h"
#include "omv_boardconfig.h"
#include "omv_csi.h"
#include "omv_crc.h"
#include "omv_protocol.h"
#include "omv_protocol_hw_caps.h"

#ifndef OMV_PROTOCOL_HW_CAPS
#define OMV_PROTOCOL_HW_CAPS    (0)
#endif

// Static global protocol context
static omv_protocol_context_t ctx;

// Validate protocol capabilities
static bool omv_protocol_check_caps(const omv_protocol_caps_t *caps) {
    return caps->max_payload >= OMV_PROTOCOL_MIN_PAYLOAD_SIZE &&
           caps->max_payload <= OMV_PROTOCOL_MAX_PAYLOAD_SIZE &&
           caps->max_retries <= OMV_PROTOCOL_MAX_RETRIES &&
           caps->lock_wait_ms >= OMV_PROTOCOL_MIN_LOCK_WAIT_MS;
}

int omv_protocol_init(const omv_protocol_channel_t *transport, const omv_protocol_caps_t *caps) {
    const omv_protocol_caps_t default_caps = {
        .crc_enabled = true,
        .seq_enabled = true,
        .ack_enabled = false,
        .frag_enabled = true,
        .max_payload = OMV_PROTOCOL_MAX_PAYLOAD_SIZE,
        .max_retries = OMV_PROTOCOL_MAX_RETRIES,
        .timeout_ms = OMV_PROTOCOL_DEFAULT_TIMEOUT_MS,
        .lock_wait_ms = OMV_PROTOCOL_MIN_LOCK_WAIT_MS,
    };

    if (transport == NULL) {
        transport = &omv_usb_channel;
    }

    if (caps == NULL) {
        caps = &default_caps;
    }

    // Validate caps parameters
    if (!omv_protocol_check_caps(caps)) {
        return -1;
    }

    // Initialize state
    ctx.sequence = 0;
    ctx.last_lock_ms = 0;
    ctx.active = true;
    ctx.wait_for_ack = false;
    ctx.state = OMV_PROTOCOL_STATE_SYNC;
    ctx.channels_count = 0; // TODO fix this and channel registration

    // Set context caps
    ctx.caps = *caps;

    // Initialize buffer
    omv_buffer_init(&ctx.buffer, ctx.rawbuf, sizeof(ctx.rawbuf));

    // Register physical channel as channel 0
    omv_protocol_register_channel(transport);
    // Register logical data channels next
    omv_protocol_register_channel(&omv_stdin_channel);
    omv_protocol_register_channel(&omv_stdout_channel);
    omv_protocol_register_channel(&omv_stream_channel);
    #if OMV_PROFILER_ENABLE
    omv_protocol_register_channel(&omv_profile_channel);
    #endif
    return 0;
}

int omv_protocol_deinit(void) {
    // Set inactive
    ctx.active = false;
    
    // Deinitialize all channels (including transport at index 0)
    for (int i = 0; i < ctx.channels_count; i++) {
        if (ctx.channels[i] && ctx.channels[i]->deinit) {
            ctx.channels[i]->deinit(ctx.channels[i]);
            ctx.channels[i] = NULL;
        }
    }
    
    return 0;
}

bool omv_protocol_active(void) {
    return ctx.active;
}

bool omv_protocol_exec_script(void) {
    const omv_protocol_channel_t *channel = omv_protocol_get_channel(OMV_PROTOCOL_CHANNEL_ID_STDIN);
    if (!channel || !channel->ioctl) {
        return false;
    }
    
    vstr_t *script = NULL;
    if (channel->ioctl(channel, 0x04, &script) != 0 || !script) {
        return false;
    }
    
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // Execute the script.
        pyexec_vstr(script, true);
        nlr_pop();
    } else {
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t) nlr.ret_val);
        // Small delay to allow the host to read the exception message
        mp_hal_delay_ms(500);
    }

    channel->ioctl(channel, 0x03, &script);
    return true;
}

int omv_protocol_register_channel(const omv_protocol_channel_t *channel) {
    if (channel->id > OMV_PROTOCOL_MAX_CHANNELS ||
        ctx.channels_count >= OMV_PROTOCOL_MAX_CHANNELS) {
        return -1;
    }
    
    // Register channel
    ctx.channels_count++;
    ctx.channels[channel->id] = channel;
    
    // Initialize the channel
    if (channel->init) {
        return channel->init(channel);
    }
    
    return 0;
}

const omv_protocol_channel_t *omv_protocol_get_channel(uint8_t channel_id) {
    if (channel_id >= OMV_PROTOCOL_MAX_CHANNELS ||
        channel_id == OMV_PROTOCOL_CHANNEL_ID_TRANSPORT) {
        return NULL;
    }

    return ctx.channels[channel_id];
}

// Calculate and check if CRC matches the one stored in buffer
static inline bool omv_protocol_crc_check(void *buf, size_t size) {
    // size includes the CRC size.
    // TODO ignore CRC for sync ?
    return size == 0 || !ctx.caps.crc_enabled || omv_crc_check(buf, size);
}

static inline bool omv_protocol_seq_check(omv_protocol_packet_t *packet) {
    return !ctx.caps.seq_enabled ||
           ctx.sequence == packet->sequence ||
           packet->opcode == OMV_PROTOCOL_OPCODE_PROTO_SYNC;
}

void omv_protocol_send_status(uint8_t opcode, omv_protocol_status_t status) {
    if (status == OMV_PROTOCOL_STATUS_SEQUENCE) {
        ctx.stats.sequence_errors++;
    } else if (status == OMV_PROTOCOL_STATUS_CHECKSUM) {
        ctx.stats.checksum_errors++;
    }

    if (status == OMV_PROTOCOL_STATUS_SUCCESS) {
        omv_protocol_send_packet(opcode, 0, NULL, OMV_PROTOCOL_FLAG_ACK);
    } else {
        omv_protocol_response_t response = {
            .status = status,
        };
        omv_protocol_send_packet(opcode, sizeof(response), &response, OMV_PROTOCOL_FLAG_NAK);
    }
}

int omv_protocol_send_packet(uint8_t opcode, size_t size, const void *data, uint8_t flags) {
    const omv_protocol_channel_t *transport = ctx.channels[OMV_PROTOCOL_CHANNEL_ID_TRANSPORT];
    if (!transport || !transport->phy_write) {
        return -1;
    }

    do {
        uint8_t crc_bytes[2];
        int retry_count = ctx.caps.max_retries;

        size_t frag_size = (size < ctx.caps.max_payload) ? size : ctx.caps.max_payload;
        uint8_t frag_flags = flags | ((size > ctx.caps.max_payload) ? OMV_PROTOCOL_FLAG_FRAGMENT : 0);
        
        // Build packet header (without CRC field initially)
        omv_protocol_packet_t packet = {
            .sync = OMV_PROTOCOL_SYNC_WORD,
            .sequence = ctx.sequence,
            .channel = 0,  // Always 0 for responses
            .flags = frag_flags,
            .opcode = opcode,
            .length = frag_size,
            .crc = 0,
        };

        // Calculate header CRC (excluding the CRC field itself)
        packet.crc = omv_crc_start(&packet, OMV_PROTOCOL_HEADER_SIZE - 2);

        // Calculate payload CRC (excluding the CRC field itself)
        if (size && data) {
            omv_crc_t crc = omv_crc_start(data, frag_size);
            crc_bytes[0] = crc & 0xFF;
            crc_bytes[1] = (crc >> 8) & 0xFF;
        }

        // Need to wait for ACK
        ctx.wait_for_ack = ctx.caps.ack_enabled && !(flags & (OMV_PROTOCOL_FLAG_ACK | OMV_PROTOCOL_FLAG_NAK));

        do {
            // Send packet: header, payload and its CRC (if any).
            int sent = 0;

            sent = transport->phy_write(transport, OMV_PROTOCOL_HEADER_SIZE, &packet, ctx.caps.timeout_ms);
            if (size && data) {
                sent += transport->phy_write(transport, frag_size, data, ctx.caps.timeout_ms);
                sent += transport->phy_write(transport, 2, crc_bytes, ctx.caps.timeout_ms);
            }

            if (transport->flush) {
                transport->flush(transport);
            }

            if (sent != OMV_PROTOCOL_HEADER_SIZE + frag_size + (frag_size > 0 ? 2 : 0)) {
                return -1;
            }

            uint32_t start_ms = mp_hal_ticks_ms();
            while (ctx.wait_for_ack && !check_timeout_ms(start_ms, ctx.caps.timeout_ms)) {
                omv_protocol_task();
                if (ctx.wait_for_ack) {
                    mp_event_handle_nowait();
                }
            }
            
            // Count retransmissions (not the first transmission)
            if (ctx.wait_for_ack && retry_count > 0) {
                ctx.stats.retransmit++;
            }
        } while (ctx.wait_for_ack && retry_count--);

        if (ctx.wait_for_ack) {
            while (1);
            return -1;
        }

        ctx.sequence++;
        ctx.stats.sent_packets++;

        if (size && data) {
            size -= frag_size;
            data = (uint8_t *)data + frag_size;
        }
    } while (size > 0);

    return 0;
}

int omv_protocol_task() {
    const omv_protocol_channel_t *transport = ctx.channels[OMV_PROTOCOL_CHANNEL_ID_TRANSPORT];
    if (!transport || !transport->size || !transport->phy_read) {
        return -1;
    }
    
    size_t available = 0;
    
    // Siphon off all available data from CDC buffer.
    while ((available = transport->size(transport))) {
        // Calculate read size: MIN(available, free_size)
        size_t free_size = omv_buffer_free(&ctx.buffer);
        size_t read_size = OMV_MIN(available, free_size);
        uint8_t *write_ptr = omv_buffer_claim(&ctx.buffer, read_size);
        
        if (!write_ptr) {
            break;
        }

        // Write data directly into buffer
        int bytes_read = transport->phy_read(transport, read_size, write_ptr, 0);
        if (bytes_read <= 0) {
            break;
        }
        
        // Commit the received data
        omv_buffer_commit(&ctx.buffer, bytes_read);
    }

    // Process state machine
    while (omv_buffer_avail(&ctx.buffer) >= OMV_PROTOCOL_SYNC_SIZE) {
        size_t buffer_size = omv_buffer_avail(&ctx.buffer);
        omv_protocol_packet_t *packet = omv_buffer_data(&ctx.buffer);

        switch (ctx.state) {
            case OMV_PROTOCOL_STATE_SYNC:
                // Look for sync pattern in the buffer
                while (omv_buffer_avail(&ctx.buffer) >= OMV_PROTOCOL_SYNC_SIZE) {
                    if (omv_buffer_peek16(&ctx.buffer) == OMV_PROTOCOL_SYNC_WORD) {
                        ctx.state = OMV_PROTOCOL_STATE_HEADER;
                        break;
                    }
                    // Consume one byte and check the next word
                    omv_buffer_consume(&ctx.buffer, 1);
                }
                break;
                
            case OMV_PROTOCOL_STATE_HEADER:
                // Check if we have a complete header
                if (buffer_size < OMV_PROTOCOL_HEADER_SIZE) {
                    return 0; // Need more data
                }

                // Validate packet header.
                ctx.state = OMV_PROTOCOL_STATE_SYNC;

                if (packet->length > ctx.caps.max_payload) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_OVERFLOW);
                } else if (!omv_protocol_seq_check(packet)) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SEQUENCE);
                } else if (!omv_protocol_crc_check(packet, OMV_PROTOCOL_HEADER_SIZE)) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_CHECKSUM);
                } else {
                    ctx.state = OMV_PROTOCOL_STATE_PAYLOAD;
                }

                // Consume a byte if the header has been rejected.
                if (ctx.state != OMV_PROTOCOL_STATE_PAYLOAD) {
                    omv_buffer_consume(&ctx.buffer, 1);
                }
                break;
                
            case OMV_PROTOCOL_STATE_PAYLOAD:             
                // HEADER + CRC + PAYLOAD + CRC
                size_t packet_size = OMV_PROTOCOL_GET_PACKET_SIZE(packet);

                // Check if we have the complete packet
                if (buffer_size < packet_size) {
                    return 0; // Need more data
                }

                // PAYLOAD + CRC
                size_t payload_size = packet_size - OMV_PROTOCOL_HEADER_SIZE;

                ctx.state = OMV_PROTOCOL_STATE_SYNC;
                omv_buffer_consume(&ctx.buffer, packet_size);

                // Check data CRC if we have payload data
                if (!omv_protocol_crc_check(packet->payload, payload_size)) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_CHECKSUM);
                } else {
                    ctx.stats.recv_packets++;
                    omv_protocol_process(packet);
                }
                break;
        }
    }
    
    return 0;
}

void omv_protocol_process(const omv_protocol_packet_t *packet) {
    if (ctx.wait_for_ack) {
        if (packet->sequence == ctx.sequence &&
            /* packet->opcode == ctx.rtx_opcode && */
            packet->flags & OMV_PROTOCOL_FLAG_ACK) {
            ctx.wait_for_ack = false;
        } else {
            // Duplicate packet? Out of order?
            while (2);
        }
        // TODO allow SYNC
        return;
    }

    switch (packet->opcode) {
        case OMV_PROTOCOL_OPCODE_SYS_RESET: {
            #if defined(OMV_BOARD_RESET)
            OMV_BOARD_RESET();
            #else
            NVIC_SystemReset();
            #endif
            break;
        }

        case OMV_PROTOCOL_OPCODE_SYS_BOOT: {
            #if defined(MICROPY_BOARD_ENTER_BOOTLOADER)
            MICROPY_BOARD_ENTER_BOOTLOADER(0, 0);
            #else
            NVIC_SystemReset();
            #endif
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_SYNC: {
            omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SUCCESS);
            ctx.sequence = 0;
            ctx.wait_for_ack = false;
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_GET_CAPS: {
            omv_protocol_send_packet(packet->opcode, sizeof(ctx.caps), &ctx.caps, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_SET_CAPS: {
            omv_protocol_caps_t *caps = (void*) packet->payload;
            if (!omv_protocol_check_caps(caps)) {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            } else {
                memcpy(&ctx.caps, caps, sizeof(ctx.caps));
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SUCCESS);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_SYS_INFO: {
            omv_protocol_sys_info_t sysinfo = { 0 };
            
            // Hardware identification
            sysinfo.cpu_id = SCB->CPUID;
            
            // Device ID from board UID
            #if (OMV_BOARD_UID_SIZE > 2)
            sysinfo.dev_id[0] = *((uint32_t *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 2));
            #endif
            sysinfo.dev_id[1] = *((uint32_t *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 1));
            sysinfo.dev_id[2] = *((uint32_t *) (OMV_BOARD_UID_ADDR + OMV_BOARD_UID_OFFSET * 0));
            
            // Camera sensor chip ID
            #if MICROPY_PY_CSI
            omv_csi_t *csi = omv_csi_get(-1);
            if (omv_csi_is_detected(csi) == true) {
                sysinfo.chip_id = omv_csi_get_id(csi);
            }
            #endif

            // Hardware capabilities
            #ifdef OMV_PROTOCOL_HW_CAPS
            sysinfo.hw_caps = OMV_PROTOCOL_HW_CAPS;
            #endif

            // Memory information
            sysinfo.flash_size_kb = 0;
            sysinfo.ram_size_kb = 0;
            sysinfo.framebuffer_size_kb = framebuffer_get(FB_MAINFB_ID)->raw_size / 1024;
            sysinfo.stream_buffer_size_kb = framebuffer_get(FB_STREAM_ID)->raw_size / 1024;
            
            // Firmware version
            sysinfo.firmware_version[0] = OMV_FIRMWARE_VERSION_MAJOR;
            sysinfo.firmware_version[1] = OMV_FIRMWARE_VERSION_MINOR;
            sysinfo.firmware_version[2] = OMV_FIRMWARE_VERSION_PATCH;

            // Protocol version
            sysinfo.protocol_version[0] = OMV_PROTOCOL_VERSION_MAJOR;
            sysinfo.protocol_version[1] = OMV_PROTOCOL_VERSION_MINOR;
            sysinfo.protocol_version[2] = OMV_PROTOCOL_VERSION_PATCH;
            
            // Bootloader version
            sysinfo.bootloader_version[0] = OMV_BOOTLOADER_VERSION_MAJOR;
            sysinfo.bootloader_version[1] = OMV_BOOTLOADER_VERSION_MINOR;
            sysinfo.bootloader_version[2] = OMV_BOOTLOADER_VERSION_PATCH;
            
            omv_protocol_send_packet(OMV_PROTOCOL_OPCODE_SYS_INFO, sizeof(sysinfo), &sysinfo, 0);
            break;
        }


        case OMV_PROTOCOL_OPCODE_CHANNEL_LIST: {
            // Build list of registered channels
            int ch_count = 0;
            omv_protocol_channel_entry_t ch_list[OMV_PROTOCOL_MAX_CHANNELS];

            for (int i = 0; i < OMV_PROTOCOL_MAX_CHANNELS; i++) {
                if (ctx.channels[i] != NULL) {
                    ch_list[ch_count].id = ctx.channels[i]->id;
                    ch_list[ch_count].flags = ctx.channels[i]->flags;
                    strncpy(ch_list[ch_count].name, ctx.channels[i]->name, OMV_PROTOCOL_CHANNEL_NAME_SIZE);
                    ch_list[ch_count].name[OMV_PROTOCOL_CHANNEL_NAME_SIZE - 1] = '\0';
                    ch_count++;
                }
            }

            size_t ch_list_size = ch_count * sizeof(omv_protocol_channel_entry_t);
            omv_protocol_send_packet(packet->opcode, ch_list_size, ch_list, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_POLL: {
            omv_protocol_channel_poll_t response = { 0 };
            
            for (int i = 0; i < OMV_PROTOCOL_MAX_CHANNELS; i++) {
                const omv_protocol_channel_t *channel = ctx.channels[i];
                if (channel && channel->poll) {
                    response.flags |= channel->poll(channel) << i;
                }
            }
            
            omv_protocol_send_packet(packet->opcode, sizeof(response), &response, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_LOCK: {
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            //if (!check_timeout_ms(ctx.last_lock_ms, ctx.caps.lock_wait_ms)) {
            //    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_BUSY);
            //    break;
            //}

            if (channel && channel->lock && channel->lock(channel) == 0) {
                ctx.last_lock_ms = mp_hal_ticks_ms();
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SUCCESS);
            } else {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_BUSY);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_UNLOCK: {
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            if (channel && channel->unlock && channel->unlock(channel) == 0) {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SUCCESS);
            } else {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_BUSY);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_SIZE: {
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            if (channel && channel->size) {
                omv_protocol_channel_size_t response;
                response.size = channel->size(channel);
                omv_protocol_send_packet(packet->opcode, sizeof(response), &response, 0);
            } else {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_SHAPE: {
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            if (channel && channel->shape) {
                size_t shape_array[4];
                size_t shape_count = channel->shape(channel, shape_array);
                omv_protocol_send_packet(packet->opcode, shape_count * sizeof(size_t), shape_array, 0);
            } else {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }


        case OMV_PROTOCOL_OPCODE_CHANNEL_READ: {
            omv_protocol_channel_request_t *request = (void*) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            if (!request->length || !channel || !(channel->read || channel->readp)) {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            } else if (channel->readp) {
                const void *data = channel->readp(channel, request->offset, request->length);
                if (!data) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_FAILED);
                } else {
                    omv_protocol_send_packet(OMV_PROTOCOL_OPCODE_CHANNEL_READ, request->length, data, 0);
                }
            } else {
                uint8_t buffer[OMV_MIN(512, ctx.caps.max_payload)];

                while (request->length > 0) {
                    size_t size_rq = OMV_MIN(request->length, sizeof(buffer));
                    int32_t size_rd = channel->read(channel, request->offset, size_rq, buffer);

                    if (size_rd <= 0) {
                        omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_FAILED);
                        break;
                    }

                    request->length -= size_rd;
                    request->offset += size_rd;
                    uint8_t flags = (request->length == 0) ? 0 : OMV_PROTOCOL_FLAG_FRAGMENT;
                    omv_protocol_send_packet(OMV_PROTOCOL_OPCODE_CHANNEL_READ, size_rd, buffer, flags);
                }
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_WRITE: {
            omv_protocol_channel_request_t *request = (void*) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            if (channel && channel->write && request->length) {
                if (channel->write(channel, request->offset, request->length, request->payload)) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SUCCESS);
                } else {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_FAILED);
                }
            } else {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_IOCTL: {
            omv_protocol_channel_ioctl_t *request = (void*) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_get_channel(packet->channel);

            if (channel && channel->ioctl) {
                if (channel->ioctl(channel, request->request, request->payload)) {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_FAILED);
                } else {
                    omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_SUCCESS);
                }
            } else {
                omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        default:
            omv_protocol_send_status(packet->opcode, OMV_PROTOCOL_STATUS_INVALID);
            break;
    }
}

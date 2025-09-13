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
#include <stddef.h>

#include "py/mphal.h"
#include "omv_common.h"
#include "omv_boardconfig.h"
#include "omv_csi.h"
#include "omv_crc.h"
#include "omv_protocol.h"
#include "omv_protocol_hw_caps.h"
#include "boot/include/version.h"

#ifndef OMV_PROTOCOL_HW_CAPS
#define OMV_PROTOCOL_HW_CAPS    (0)
#endif

// Static global protocol context
static omv_protocol_context_t ctx;

static const omv_protocol_config_t default_config = {
    .crc_enabled = true,
    .seq_enabled = true,
    .ack_enabled = false,
    .event_enabled = true,
    .max_payload = OMV_PROTOCOL_MAX_PAYLOAD_SIZE,
    .soft_reboot = true,
    .retry_count = OMV_PROTOCOL_MAX_RETRY_COUNT,
    .timeout_ms = OMV_PROTOCOL_DEFAULT_TIMEOUT_MS,
    .lock_wait_ms = OMV_PROTOCOL_MIN_LOCK_WAIT_MS,
};

int omv_protocol_init(const omv_protocol_config_t *config) {
    if (!config) {
        return -1;
    }

    // Validate config
    if (config->max_payload < OMV_PROTOCOL_MIN_PAYLOAD_SIZE ||
        config->max_payload > OMV_PROTOCOL_MAX_PAYLOAD_SIZE ||
        config->retry_count > OMV_PROTOCOL_MAX_RETRY_COUNT ||
        config->lock_wait_ms < OMV_PROTOCOL_MIN_LOCK_WAIT_MS) {
        return -1;
    }

    // Initialize state
    ctx.config = *config;
    ctx.sequence = 0;
    ctx.last_lock_ms = 0;
    ctx.state = OMV_PROTOCOL_STATE_SYNC;    
    ctx.channels_count = 0;
    memset(ctx.channels, 0, sizeof(ctx.channels));

    // Initialize ACK queue
    omv_protocol_queue_reset(&ctx.ack_queue);

    // Initialize buffer
    omv_buffer_init(&ctx.buffer, ctx.rawbuf, sizeof(ctx.rawbuf));

    return 0;
}

int omv_protocol_init_default(void) {
    if (omv_protocol_init(&default_config) != 0) {
        return -1;
    }

    #if OMV_PROTOCOL_DEFAULT_CHANNELS
    // Register the physical transport as channel 0
    omv_protocol_register_channel(&omv_usb_channel);

    // Register the default logical data channels next
    omv_protocol_register_channel(&omv_stdin_channel);
    omv_protocol_register_channel(&omv_stdout_channel);
    omv_protocol_register_channel(&omv_stream_channel);

    // Register the profiler channel (if enabled)
    #if OMV_PROFILER_ENABLE
    omv_protocol_register_channel(&omv_profile_channel);
    #endif // OMV_PROFILER_ENABLE

    #endif // OMV_PROTOCOL_DEFAULT_CHANNELS
    
    return 0;
}

int omv_protocol_deinit(void) {
    // Deinitialize all channels (including transport at index 0)
    for (int i = 0; i < ctx.channels_count; i++) {
        if (ctx.channels[i] && ctx.channels[i]->deinit) {
            ctx.channels[i]->deinit(ctx.channels[i]);
            ctx.channels[i] = NULL;
        }
    }
    return 0;
}

void omv_protocol_reset_state(void) {
    ctx.sequence = 0;
    ctx.config = default_config;
    ctx.state = OMV_PROTOCOL_STATE_SYNC;    
    omv_buffer_clear(&ctx.buffer);
    omv_protocol_queue_reset(&ctx.ack_queue);
    
    // Unlock any locked channels when losing sync
    for (int i = 0; i < ctx.channels_count; i++) {
        const omv_protocol_channel_t *channel = ctx.channels[i];
        if (channel && channel->unlock) {
            channel->unlock(channel);
        }
    }
}

bool omv_protocol_is_active(void) {
    const omv_protocol_channel_t *transport = omv_protocol_find_transport();
    return transport && transport->is_active(transport);
}

bool omv_protocol_exec_script(void) {
    const omv_protocol_channel_t *channel = omv_protocol_find_channel(OMV_PROTOCOL_CHANNEL_ID_STDIN);
    if (!channel || !channel->exec) {
        return false;
    }
    
    bool result = channel->exec(channel);
    if (result) {
        omv_protocol_send_event(0, OMV_PROTOCOL_EVENT_SOFT_REBOOT);
    }

    if (result) {
        // A script was executed - return true if the transport allows soft-reboot
        return ctx.config.soft_reboot;
    }

    return false;
}

int omv_protocol_register_channel(const omv_protocol_channel_t *channel) {   
    // Check max channels
    if (ctx.channels_count >= OMV_PROTOCOL_MAX_CHANNELS) {
        return -1;
    }

    // The physical transport channel must be registered first
    if (!omv_protocol_find_transport() &&
        !OMV_PROTOCOL_CHANNEL_IS_TRANSPORT(channel)) {
        return -1;
    }
 
    // Initialize the channel
    if (channel->init && channel->init(channel)) {
        return -1;
    }
 
    // Register channel at next available index
    ctx.channels[ctx.channels_count] = channel;

    // Send channel registered event to host
    if (OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, DYNAMIC)) {
        ((omv_protocol_channel_t *) channel)->id = ctx.channels_count;
        omv_protocol_send_event(0, OMV_PROTOCOL_EVENT_CHANNEL_REGISTERED);
    }

    return ctx.channels_count++;
}

// Find and verify transport channel
const omv_protocol_channel_t *omv_protocol_find_transport(void) {
    const omv_protocol_channel_t *transport = ctx.channels[OMV_PROTOCOL_CHANNEL_ID_TRANSPORT];
    return (transport && OMV_PROTOCOL_CHANNEL_IS_TRANSPORT(transport)) ? transport : NULL;
}

const omv_protocol_channel_t *omv_protocol_find_channel(uint8_t channel_id) {
    if (channel_id >= ctx.channels_count) {
        return NULL;
    }

    return ctx.channels[channel_id];
}

// Calculate and check if CRC matches the one stored in buffer
static inline bool omv_protocol_crc_check(void *buf, size_t size) {
    return !size || !ctx.config.crc_enabled || omv_crc_check(buf, size);
}

// Protocol and system commands and events must arrive on channel 0
static bool omv_protocol_channel_check(const omv_protocol_packet_t *packet) {
    return packet->channel == 0 || packet->opcode > OMV_PROTOCOL_OPCODE_SYS_LAST;
}

static inline bool omv_protocol_seq_check(omv_protocol_packet_t *packet) {
    return !ctx.config.seq_enabled ||
           ctx.sequence == packet->sequence ||
           packet->opcode == OMV_PROTOCOL_OPCODE_SYS_EVENT ||
           packet->opcode == OMV_PROTOCOL_OPCODE_CHANNEL_EVENT ||
           packet->opcode == OMV_PROTOCOL_OPCODE_PROTO_SYNC;
}

int omv_protocol_send_event(uint8_t channel_id, uint16_t event) {
    uint32_t flags = OMV_PROTOCOL_FLAG_EVENT;

    if (!ctx.config.event_enabled ||
        !omv_protocol_is_active() ||
        omv_protocol_queue_is_full(&ctx.ack_queue)) {
        return -1;
    }

    ctx.stats.sent_events++;
    uint8_t opcode = (channel_id == 0) ? OMV_PROTOCOL_OPCODE_SYS_EVENT: OMV_PROTOCOL_OPCODE_CHANNEL_EVENT;
    
    if (event == OMV_PROTOCOL_EVENT_NOTIFY) {
        return omv_protocol_send_packet(opcode, channel_id, 0, NULL, flags);
    } else {
        return omv_protocol_send_packet(opcode, channel_id, sizeof(event), &event, flags);
    }
}

static void omv_protocol_send_status(uint8_t opcode, uint8_t channel_id, omv_protocol_status_t status) {
    if (status == OMV_PROTOCOL_STATUS_SEQUENCE) {
        ctx.stats.sequence_errors++;
    } else if (status == OMV_PROTOCOL_STATUS_CHECKSUM) {
        ctx.stats.checksum_errors++;
    }

    if (status == OMV_PROTOCOL_STATUS_SUCCESS) {
        omv_protocol_send_packet(opcode, channel_id, 0, NULL, OMV_PROTOCOL_FLAG_ACK);
    } else {
        omv_protocol_response_t response = {
            .status = status,
        };
        omv_protocol_send_packet(opcode, channel_id, sizeof(response), &response, OMV_PROTOCOL_FLAG_NAK);
    }
}

int omv_protocol_send_packet(uint8_t opcode, uint8_t channel_id, size_t size, const void *data, uint8_t flags) {
    const omv_protocol_channel_t *transport = omv_protocol_find_transport();
    if (!transport || !transport->is_active(transport)) {
        return -1;
    }

    do {
        uint8_t crc16_bytes[2];
        int retry_count = ctx.config.retry_count;

        size_t frag_size = (size < ctx.config.max_payload) ? size : ctx.config.max_payload;
        uint8_t frag_flags = flags | ((size > ctx.config.max_payload) ? OMV_PROTOCOL_FLAG_FRAGMENT : 0);
        bool wait_for_ack = ctx.config.ack_enabled && !(flags & (OMV_PROTOCOL_FLAG_ACK | OMV_PROTOCOL_FLAG_NAK));
        
        // Build packet header
        omv_protocol_packet_t packet = {
            .sync = OMV_PROTOCOL_SYNC_WORD,
            .sequence = ctx.sequence,
            .channel = channel_id,
            .flags = frag_flags,
            .opcode = opcode,
            .length = frag_size,
        };

        // Calculate header CRC (excluding the CRC field itself)
        packet.crc = omv_crc_start(&packet, offsetof(omv_protocol_packet_t, crc));

        // Calculate payload CRC (excluding the CRC field itself)
        if (size && data) {
            omv_crc_t crc = omv_crc_start(data, frag_size);
            crc16_bytes[0] = crc & 0xFF;
            crc16_bytes[1] = (crc >> 8) & 0xFF;
        }

        // Push ACK packet on the queue.
        if (wait_for_ack) {
            if (!omv_protocol_queue_push(&ctx.ack_queue, packet.opcode, packet.sequence)) {
                return -1;  // Queue full
            }
            // Update max ACK queue depth stat
            if (ctx.ack_queue.count > ctx.stats.max_ack_queue_depth) {
                ctx.stats.max_ack_queue_depth = ctx.ack_queue.count;
            }
        }

        do {
            // Send packet: header, payload and its CRC (if any).
            int sent = 0;

            sent = transport->phy_write(transport, OMV_PROTOCOL_HEADER_SIZE, &packet, ctx.config.timeout_ms);
            if (size && data) {
                sent += transport->phy_write(transport, frag_size, data, ctx.config.timeout_ms);
                sent += transport->phy_write(transport, 2, crc16_bytes, ctx.config.timeout_ms);
            }

            if (transport->flush) {
                transport->flush(transport);
            }

            if (sent != OMV_PROTOCOL_HEADER_SIZE + frag_size + (frag_size > 0 ? 2 : 0)) {
                ctx.stats.transport_errors++;
                return -1;
            }

            for (uint32_t start = mp_hal_ticks_ms(); wait_for_ack; mp_event_handle_nowait()) {
                if (omv_protocol_task() == -1) {
                    return -1;
                }
               
                // Check if our ACK was received (no longer in queue)
                wait_for_ack = omv_protocol_queue_check(&ctx.ack_queue, opcode, ctx.sequence);

                if (check_timeout_ms(start, ctx.config.timeout_ms)) {
                    if (!retry_count) {
                        return -1;
                    }
                    ctx.stats.retransmit++;
                    break;
                }
            }
        } while (wait_for_ack && retry_count--);

        if (size && data) {
            size -= frag_size;
            data = (uint8_t *)data + frag_size;
        }

        ctx.stats.sent_packets++;
        if (!(flags & OMV_PROTOCOL_FLAG_EVENT)) {
            ctx.sequence++;
        }
    } while (size > 0);

    return 0;
}

int omv_protocol_task(void) {
    size_t available = 0;
    const omv_protocol_channel_t *transport = omv_protocol_find_transport();

    if (!transport || !transport->is_active(transport)) {
        return -1;
    }

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
        // TODO add timeout.
        int bytes_read = transport->phy_read(transport, read_size, write_ptr, 0);
        if (bytes_read <= 0) {
            break;
        }
        
        // Commit the received data
        omv_buffer_commit(&ctx.buffer, bytes_read);
    }

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

                if (packet->length > ctx.config.max_payload) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_OVERFLOW);
                } else if (!omv_protocol_seq_check(packet)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SEQUENCE);
                } else if (!omv_protocol_crc_check(packet, OMV_PROTOCOL_HEADER_SIZE)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_CHECKSUM);
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
                size_t payload_size = packet_size - OMV_PROTOCOL_HEADER_SIZE;

                // Check if we have the complete packet
                if (buffer_size < packet_size) {
                    return 0; // Need more data
                }

                ctx.state = OMV_PROTOCOL_STATE_SYNC;
                // protocol_process may send a packet that expects an ACK.
                // Clear the buffer first before calling protocol_process.
                omv_buffer_consume(&ctx.buffer, packet_size);

                // Check data CRC if we have payload data
                if (!omv_protocol_crc_check(packet->payload, payload_size)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_CHECKSUM);
                } else if (!omv_protocol_channel_check(packet)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
                } else if (packet->flags & OMV_PROTOCOL_FLAG_ACK) {
                    // ACK processed - removed from queue
                    if (!omv_protocol_queue_pop(&ctx.ack_queue, packet->opcode,
                                                packet->sequence, ctx.config.seq_enabled)) {
                        // Unexpected ACK.
                        omv_protocol_reset_state();
                        return -1;
                    }
                } else {
                    omv_protocol_process(packet);
                }

                break;
        }
    }
    return 0;
}

void omv_protocol_process(const omv_protocol_packet_t *packet) {
    ctx.stats.recv_packets++;

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
            ctx.sequence = 0;
            omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SUCCESS);
            omv_protocol_reset_state();
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_GET_CAPS: {
            // Convert internal config to wire caps format
            omv_protocol_caps_t caps = {0};
            caps.crc_enabled = ctx.config.crc_enabled;
            caps.seq_enabled = ctx.config.seq_enabled;
            caps.ack_enabled = ctx.config.ack_enabled;
            caps.event_enabled = ctx.config.event_enabled;
            caps.max_payload = ctx.config.max_payload;
            // Transport fields are not sent over wire
            omv_protocol_send_packet(packet->opcode, packet->channel, sizeof(caps), &caps, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_SET_CAPS: {
            omv_protocol_caps_t *caps = (void*) packet->payload;
            // Validate only the protocol capability fields
            if (caps->max_payload < OMV_PROTOCOL_MIN_PAYLOAD_SIZE ||
                caps->max_payload > OMV_PROTOCOL_MAX_PAYLOAD_SIZE) {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
            } else {
                // Update only protocol capability fields, preserve transport config
                ctx.config.crc_enabled = caps->crc_enabled;
                ctx.config.seq_enabled = caps->seq_enabled;
                ctx.config.ack_enabled = caps->ack_enabled;
                ctx.config.event_enabled = caps->event_enabled;
                ctx.config.max_payload = caps->max_payload;
                // Transport config fields remain unchanged
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SUCCESS);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_STATS: {
            omv_protocol_send_packet(packet->opcode, packet->channel, sizeof(ctx.stats), &ctx.stats, 0);
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
            size_t chip_count = 0;
            size_t max_chip_ids = OMV_ARRAY_SIZE(sysinfo.chip_id);
            for (size_t i=0; chip_count < max_chip_ids && i < OMV_CSI_MAX_DEVICES; i++) {
                omv_csi_t *csi = &csi_all[i];
                if (csi->detected) {
                    sysinfo.chip_id[chip_count++] = omv_csi_get_id(csi);
                }
            }
            #endif

            // Hardware capabilities
            #ifdef OMV_PROTOCOL_HW_CAPS
            sysinfo.hw_caps[0] = OMV_PROTOCOL_HW_CAPS;
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
            
            omv_protocol_send_packet(OMV_PROTOCOL_OPCODE_SYS_INFO, packet->channel, sizeof(sysinfo), &sysinfo, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_LIST: {
            // Build list of registered channels
            int ch_count = 0;
            omv_protocol_channel_entry_t ch_list[OMV_PROTOCOL_MAX_CHANNELS];

            for (int i = 0; i < ctx.channels_count; i++) {
                if (ctx.channels[i] != NULL) {
                    ch_list[ch_count].id = ctx.channels[i]->id;
                    ch_list[ch_count].flags = ctx.channels[i]->flags;
                    strncpy(ch_list[ch_count].name, ctx.channels[i]->name, OMV_PROTOCOL_CHANNEL_NAME_SIZE);
                    ch_list[ch_count].name[OMV_PROTOCOL_CHANNEL_NAME_SIZE - 1] = '\0';
                    ch_count++;
                }
            }

            size_t ch_list_size = ch_count * sizeof(omv_protocol_channel_entry_t);
            omv_protocol_send_packet(packet->opcode, packet->channel, ch_list_size, ch_list, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_POLL: {
            omv_protocol_channel_poll_t response = { 0 };
            
            for (int i = 0; i < ctx.channels_count; i++) {
                const omv_protocol_channel_t *channel = ctx.channels[i];
                if (channel && channel->poll) {
                    response.flags |= channel->poll(channel) << i;
                }
            }
            
            omv_protocol_send_packet(packet->opcode, packet->channel, sizeof(response), &response, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_LOCK: {
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (!check_timeout_ms(ctx.last_lock_ms, ctx.config.lock_wait_ms)) {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_BUSY);
            } else if (channel && channel->lock && channel->lock(channel) == 0) {
                ctx.last_lock_ms = mp_hal_ticks_ms();
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SUCCESS);
            } else {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_BUSY);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_UNLOCK: {
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->unlock && channel->unlock(channel) == 0) {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SUCCESS);
            } else {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_BUSY);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_SIZE: {
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->size) {
                omv_protocol_channel_size_t response;
                response.size = channel->size(channel);
                omv_protocol_send_packet(packet->opcode, packet->channel, sizeof(response), &response, 0);
            } else {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_SHAPE: {
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->shape) {
                size_t shape_array[4];
                size_t shape_size = channel->shape(channel, shape_array) * sizeof(size_t);
                omv_protocol_send_packet(packet->opcode, packet->channel, shape_size, shape_array, 0);
            } else {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_READ: {
            omv_protocol_channel_io_t *request = (void*) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (!request->length || !channel || !(channel->read || channel->readp)) {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
            } else if (channel->readp) {
                const void *data = channel->readp(channel, request->offset, request->length);
                if (!data) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_FAILED);
                } else {
                    omv_protocol_send_packet(packet->opcode, packet->channel, request->length, data, 0);
                }
            } else {
                uint8_t buffer[OMV_MIN(512, ctx.config.max_payload)];

                while (request->length > 0) {
                    size_t size_rq = OMV_MIN(request->length, sizeof(buffer));
                    int32_t size_rd = channel->read(channel, request->offset, size_rq, buffer);

                    if (size_rd <= 0) {
                        omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_FAILED);
                        break;
                    }

                    request->length -= size_rd;
                    request->offset += size_rd;
                    uint8_t flags = (request->length == 0) ? 0 : OMV_PROTOCOL_FLAG_FRAGMENT;
                    omv_protocol_send_packet(packet->opcode, packet->channel, size_rd, buffer, flags);
                }
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_WRITE: {
            omv_protocol_channel_io_t *request = (void*) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->write && request->length) {
                if (channel->write(channel, request->offset, request->length, request->payload)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SUCCESS);
                } else {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_FAILED);
                }
            } else {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_IOCTL: {
            omv_protocol_channel_ioctl_t *ioctl = (void*) packet->payload;
            size_t ioctl_len = packet->length - offsetof(omv_protocol_channel_ioctl_t, payload);
            uint8_t *ioctl_arg = (ioctl_len) ? ioctl->payload : NULL;
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->ioctl) {
                // Validate argument size for static channels only
                if (!OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, DYNAMIC) &&
                    !omv_protocol_ioctl_check(packet->channel, ioctl->request, ioctl_len)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
                } else if (channel->ioctl(channel, ioctl->request, ioctl_len, ioctl_arg)) {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_FAILED);
                } else {
                    omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_SUCCESS);
                }
            } else {
                omv_protocol_send_status(packet->opcode, packet->channel, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        default:
            omv_protocol_send_status(packet->opcode, 0, OMV_PROTOCOL_STATUS_INVALID);
            break;
    }
}

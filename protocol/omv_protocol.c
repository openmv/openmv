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
#include "omv_csi.h"
#include "omv_crc.h"
#include "omv_protocol.h"
#include "omv_protocol_hw_caps.h"
#include "boot/include/header.h"

#ifndef OMV_PROTOCOL_HW_CAPS
#define OMV_PROTOCOL_HW_CAPS    (0)
#endif

// Static global protocol context
static omv_protocol_context_t ctx;
static omv_protocol_config_t default_config;

int omv_protocol_init(const omv_protocol_config_t *config) {
    if (!config) {
        return -1;
    }

    // Validate config
    if (config->max_payload < OMV_PROTOCOL_MIN_PAYLOAD_SIZE ||
        config->max_payload > OMV_PROTOCOL_MAX_PAYLOAD_SIZE ||
        config->lock_intval_ms < OMV_PROTOCOL_MIN_LOCK_INTERVAL_MS) {
        return -1;
    }

    // Initialize state
    ctx.sequence = 0;
    ctx.last_lock_ms = 0;
    ctx.scan_offset = 0;
    ctx.wait_for_ack = false;
    ctx.state = OMV_PROTOCOL_STATE_SYNC;
    ctx.channels_count = 0;
    memset(ctx.channels, 0, sizeof(ctx.channels));

    // Use the config provided by this transport
    // For USB, see defaults below. Uart could enable CRC, ACKs etc..
    ctx.config = *config;

    // Save as default config, which will be restored on re-sync
    default_config = *config;

    // Initialize buffer
    omv_buffer_init(&ctx.buffer, ctx.rawbuf, sizeof(ctx.rawbuf));

    return 0;
}

int omv_protocol_init_default() {
    const omv_protocol_config_t config = {
        .crc_enabled = true,
        .seq_enabled = true,
        .ack_enabled = true,
        .event_enabled = true,
        .max_payload = OMV_PROTOCOL_MAX_PAYLOAD_SIZE,
        .soft_reboot = true,
        .rtx_retries = OMV_PROTOCOL_DEF_RTX_RETRIES,
        .rtx_timeout_ms = OMV_PROTOCOL_DEF_RTX_TIMEOUT_MS,
        .lock_intval_ms = OMV_PROTOCOL_MIN_LOCK_INTERVAL_MS,
    };

    if (omv_protocol_init(&config) != 0) {
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

void omv_protocol_deinit(void) {
    // Deinitialize all channels (including transport at index 0)
    for (int i = 0; i < OMV_PROTOCOL_MAX_CHANNELS; i++) {
        if (ctx.channels[i] && ctx.channels[i]->deinit) {
            ctx.channels[i]->deinit(ctx.channels[i]);
            ctx.channels[i] = NULL;
        }
    }
}

void omv_protocol_reset(void) {
    // Reset state
    ctx.sequence = 0;
    ctx.scan_offset = 0;
    ctx.wait_for_ack = false;
    ctx.state = OMV_PROTOCOL_STATE_SYNC;
    omv_buffer_clear(&ctx.buffer);

    // Restore default config
    ctx.config = default_config;

    // Unlock channels
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
        omv_protocol_send_event(0, OMV_PROTOCOL_EVENT_SOFT_REBOOT, false);
    }

    if (result) {
        // A script was executed - return true if the transport allows soft-reboot
        return ctx.config.soft_reboot;
    }

    return false;
}

int omv_protocol_register_channel(const omv_protocol_channel_t *channel) {
    int channel_id = -1;

    if (OMV_PROTOCOL_CHANNEL_IS_TRANSPORT(channel)) {
        channel_id = 0;
    } else if (!OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, DYNAMIC)) {
        // Use statically defined channel ID
        channel_id = channel->id;
    } else {
        // Find the first free channel
        for (size_t i = 1; i < OMV_PROTOCOL_MAX_CHANNELS; i++) {
            if (ctx.channels[i] == NULL) {
                channel_id = i;
                break;
            }
        }
    }

    // Initialize the channel
    if (channel->init && channel->init(channel)) {
        return -1;
    }

    // Register channel at next available index
    ctx.channels[channel_id] = channel;

    // Send channel registered event to host
    if (OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, DYNAMIC)) {
        ((omv_protocol_channel_t *) channel)->id = channel_id;
        omv_protocol_send_event(0, OMV_PROTOCOL_EVENT_CHANNEL_REGISTERED, false);
    }

    // If no physical transport is ever registered, the count will be one
    // less than the number of channels as they're offset by 1. However,
    // channels_count is only used when the physical transport is active.
    ctx.channels_count++;

    return channel_id;
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
static inline bool omv_protocol_crc_check(omv_crc_size_t crc, void *buf, size_t size) {
    return !size || !ctx.config.crc_enabled || omv_crc_check(crc, buf, size);
}

// Protocol and system commands and events must arrive on channel 0
static bool omv_protocol_channel_check(const omv_protocol_packet_t *packet) {
    return packet->channel == 0 || packet->opcode > OMV_PROTOCOL_OPCODE_SYS_LAST;
}

static inline bool omv_protocol_seq_check(omv_protocol_packet_t *packet) {
    return !ctx.config.seq_enabled ||
           ctx.sequence == packet->sequence ||
           (packet->flags & (OMV_PROTOCOL_FLAG_ACK | OMV_PROTOCOL_FLAG_NAK)) ||
           packet->opcode == OMV_PROTOCOL_OPCODE_SYS_EVENT ||
           packet->opcode == OMV_PROTOCOL_OPCODE_CHANNEL_EVENT ||
           packet->opcode == OMV_PROTOCOL_OPCODE_PROTO_SYNC;
}

static bool omv_protocol_ioctl_check(uint8_t channel_id, uint32_t cmd, size_t len) {
    static const struct {
        uint8_t ch; uint32_t cmd; size_t size;
    } ioctl_table[] = {
        OMV_PROTOCOL_CHANNEL_IOCTL_TABLE
    };

    for (int i = 0; i < sizeof(ioctl_table) / sizeof(ioctl_table[0]); i++) {
        if (ioctl_table[i].ch == channel_id && ioctl_table[i].cmd == cmd) {
            return len == ioctl_table[i].size;
        }
    }
    return false; // Unknown ioctl on static channel
}

// Check if packet is a valid SYNC command
static inline bool omv_protocol_is_sync(const omv_protocol_packet_t *packet) {
    return packet->sync == OMV_PROTOCOL_SYNC_WORD &&
           packet->channel == 0 && packet->length == 0 &&
           packet->opcode == OMV_PROTOCOL_OPCODE_PROTO_SYNC &&
           omv_protocol_crc_check(OMV_CRC16, (void *) packet, OMV_PROTOCOL_HEADER_SIZE);
}

// Check if packet is a valid ACK for expected opcode/sequence
static inline bool omv_protocol_is_ack(const omv_protocol_packet_t *packet, uint8_t opcode, uint8_t sequence) {
    return packet->sync == OMV_PROTOCOL_SYNC_WORD &&
           (packet->flags & (OMV_PROTOCOL_FLAG_ACK | OMV_PROTOCOL_FLAG_ACK)) &&
           packet->opcode == opcode && packet->sequence == sequence &&
           omv_protocol_crc_check(OMV_CRC16, (void *) packet, OMV_PROTOCOL_HEADER_SIZE);
}

static void omv_protocol_send_status(const omv_protocol_packet_t *packet, omv_protocol_status_t status) {
    if (status == OMV_PROTOCOL_STATUS_SEQUENCE) {
        ctx.stats.sequence_errors++;
    } else if (status == OMV_PROTOCOL_STATUS_CHECKSUM) {
        ctx.stats.checksum_errors++;
    }

    if (status == OMV_PROTOCOL_STATUS_SUCCESS) {
        omv_protocol_send_packet(packet->opcode, packet->channel, 0, NULL, OMV_PROTOCOL_FLAG_ACK);
    } else {
        omv_protocol_response_t resp = {
            .status = status,
        };
        omv_protocol_send_packet(packet->opcode, packet->channel, sizeof(resp), &resp, OMV_PROTOCOL_FLAG_NAK);
    }
}

int omv_protocol_send_event(uint8_t channel_id, uint16_t event, bool wait_ack) {
    int ret = 0;

    if (!ctx.config.event_enabled || !omv_protocol_is_active()) {
        return -1;
    }

    uint32_t flags = OMV_PROTOCOL_FLAG_EVENT | (wait_ack ? OMV_PROTOCOL_FLAG_ACK_REQ : 0);
    uint8_t opcode = (channel_id == 0) ? OMV_PROTOCOL_OPCODE_SYS_EVENT: OMV_PROTOCOL_OPCODE_CHANNEL_EVENT;

    ctx.stats.sent_events++;
    if (event == OMV_PROTOCOL_EVENT_NOTIFY) {
        ret = omv_protocol_send_packet(opcode, channel_id, 0, NULL, flags);
    } else {
        ret = omv_protocol_send_packet(opcode, channel_id, sizeof(event), &event, flags);
    }

    // The state machine returns immediately after finding an event ACK so we need
    // to run it one more time to handle any buffered commands. This is especially
    // important for the USB transport, as it schedules the task on receive IRQs,
    // which may not occur again if the host is waiting on a reply.
    if (ret != -1 && wait_ack) {
        omv_protocol_task();
    }

    return ret;
}

int omv_protocol_send_packet(uint8_t opcode, uint8_t channel_id, size_t size, const void *data, uint8_t flags) {
    const omv_protocol_channel_t *transport = omv_protocol_find_transport();
    if (!transport || !transport->is_active(transport)) {
        return -1;
    }

    if (!ctx.config.ack_enabled) {
        // ACK is disabled globally
        flags &= ~OMV_PROTOCOL_FLAG_ACK_REQ;
    } else if (!(flags & OMV_PROTOCOL_FLAG_NO_ACK)) {
        flags |= OMV_PROTOCOL_FLAG_ACK_REQ;
    }

    do {
        int rtx_retries = ctx.config.rtx_retries;
        uint32_t rtx_timeout = ctx.config.rtx_timeout_ms;

        uint8_t crc32_bytes[4];
        size_t frag_len = (size <= ctx.config.max_payload) ? size : ctx.config.max_payload;
        uint8_t frag_flags = (size <= ctx.config.max_payload) ? flags : (flags | OMV_PROTOCOL_FLAG_FRAGMENT);

        // Build packet header
        omv_protocol_packet_t packet = {
            .sync = OMV_PROTOCOL_SYNC_WORD,
            .sequence = ctx.sequence,
            .channel = channel_id,
            .flags = frag_flags,
            .opcode = opcode,
            .length = frag_len,
        };

        // Calculate header CRC (excluding the CRC field itself)
        if (ctx.config.crc_enabled) {
            packet.crc = omv_crc_start(OMV_CRC16, &packet, OMV_PROTOCOL_HEADER_SIZE - 2);
        }

        // Calculate payload CRC (excluding the CRC field itself)
        if (ctx.config.crc_enabled && size && data) {
            *((uint32_t *) crc32_bytes) = omv_crc_start(OMV_CRC32, data, frag_len);
        }

        // Set up ACK waiting context
        if (flags & OMV_PROTOCOL_FLAG_ACK_REQ) {
            ctx.scan_offset = 0;
            ctx.ack_status = -1;
            ctx.ack_opcode = packet.opcode;
            ctx.ack_sequence = packet.sequence;
            ctx.wait_for_ack = true;
        }

        do {
            // Send packet header, payload and CRC
            int sent = transport->write(transport, 0, OMV_PROTOCOL_HEADER_SIZE, &packet);
            if (size && data) {
                sent += transport->write(transport, 0, frag_len, data);
                sent += transport->write(transport, 0, 4, crc32_bytes);
            }

            if (transport->flush) {
                transport->flush(transport);
            }

            if (sent != OMV_PROTOCOL_HEADER_SIZE + frag_len + (frag_len > 0 ? 4 : 0)) {
                ctx.stats.transport_errors++;
                return -1;
            }

            for (uint32_t start = OMV_PROTOCOL_TICKS_MS(); ctx.wait_for_ack; OMV_PROTOCOL_EVENT_POLL()) {
                if (omv_protocol_task() == -1) {
                    return -1;
                }

                if (ctx.ack_status == OMV_PROTOCOL_STATUS_SUCCESS) {
                    break;
                }

                // Received NACK or timeout
                if (ctx.ack_status == OMV_PROTOCOL_STATUS_FAILED ||
                    omv_protocol_check_timeout(start, rtx_timeout)) {
                    if (rtx_retries-- <= 0) {
                        omv_protocol_reset();
                        return -1;
                    }

                    // Double RTX timeout
                    rtx_timeout *= 2;
                    ctx.stats.retransmit++;

                    // Set RTX and recalculate the CRC.
                    if (ctx.config.crc_enabled && !(packet.flags & OMV_PROTOCOL_FLAG_RTX)) {
                        packet.flags |= OMV_PROTOCOL_FLAG_RTX;
                        packet.crc = omv_crc_start(OMV_CRC16, &packet, OMV_PROTOCOL_HEADER_SIZE - 2);
                    }
                    break;
                }
            }
        } while (ctx.wait_for_ack);

        if (size && data) {
            size -= frag_len;
            data = (uint8_t *) data + frag_len;
        }

        if (!(flags & OMV_PROTOCOL_FLAG_EVENT)) {
            ctx.sequence++;
            ctx.stats.sent_packets++;
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
        int bytes_read = transport->read(transport, 0, read_size, write_ptr);
        if (bytes_read <= 0) {
            break;
        }

        // Commit the received data
        omv_buffer_commit(&ctx.buffer, bytes_read);
    }

    while (omv_buffer_avail(&ctx.buffer) >= OMV_PROTOCOL_SYNC_SIZE) {
        uint8_t *buffer = omv_buffer_data(&ctx.buffer);
        int32_t buffer_size = omv_buffer_avail(&ctx.buffer);
        omv_protocol_packet_t *packet = omv_buffer_data(&ctx.buffer);

        switch (ctx.wait_for_ack ? OMV_PROTOCOL_STATE_WAIT_ACK : ctx.state) {
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
                if (!omv_protocol_crc_check(OMV_CRC16, packet, OMV_PROTOCOL_HEADER_SIZE)) {
                    // No further validation needed
                } else if (packet->length > ctx.config.max_payload) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_OVERFLOW);
                } else if (!omv_protocol_seq_check(packet)) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SEQUENCE);
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
                size_t packet_size = OMV_PROTOCOL_PACKET_GET_SIZE(packet);
                size_t payload_size = packet_size - OMV_PROTOCOL_HEADER_SIZE;

                // Check if we have the complete packet
                if (buffer_size < packet_size) {
                    // Transition to SYNC_RECOVERY to scan for SYNC commands
                    ctx.scan_offset = 0;
                    ctx.state = OMV_PROTOCOL_STATE_SYNC_RECOVERY;
                    break;
                }

                ctx.state = OMV_PROTOCOL_STATE_SYNC;
                // protocol_process may send a packet that expects an ACK.
                // Clear the buffer first before calling protocol_process.
                omv_buffer_consume(&ctx.buffer, packet_size);

                // Check data CRC if we have payload data
                if (!omv_protocol_crc_check(OMV_CRC32, packet->payload, payload_size)) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_CHECKSUM);
                } else if (!omv_protocol_channel_check(packet)) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
                } else if (packet->flags & (OMV_PROTOCOL_FLAG_ACK | OMV_PROTOCOL_FLAG_NAK)) {
                    // ACK/NAK packets are handled by WAIT_ACK state - ignore here
                } else {
                    omv_protocol_process(packet);
                }

                break;

            // Scan through the buffer looking for a complete SYNC command
            // Prevents a partial packet from deadlocking the state machine
            case OMV_PROTOCOL_STATE_SYNC_RECOVERY:
                if (buffer_size >= OMV_PROTOCOL_PACKET_GET_SIZE(packet)) {
                    ctx.state = OMV_PROTOCOL_STATE_SYNC;
                    break;
                }

                for (; ctx.scan_offset <= buffer_size - OMV_PROTOCOL_HEADER_SIZE; ctx.scan_offset++) {
                    omv_protocol_packet_t *packet = (omv_protocol_packet_t *) (buffer + ctx.scan_offset);

                    if (omv_protocol_is_sync(packet)) {
                        // SYNC command is found - process SYNC and reset state
                        omv_protocol_process(packet);
                        return 0;
                    }
                }

                // No SYNC command found - go back to PAYLOAD state
                ctx.state = OMV_PROTOCOL_STATE_PAYLOAD;
                return 0;

            // Scan through buffer looking for expected ACK packet
            case OMV_PROTOCOL_STATE_WAIT_ACK:
                for (; ctx.scan_offset <= buffer_size - OMV_PROTOCOL_HEADER_SIZE; ctx.scan_offset++) {
                    omv_protocol_packet_t *packet = (omv_protocol_packet_t *) (buffer + ctx.scan_offset);

                    // SYNC found while waiting for ACK
                    if (omv_protocol_is_sync(packet)) {
                        omv_protocol_process(packet);
                        return -1;
                    }

                    // Found the matching ACK/NAK - consume it and return to SYNC
                    if (omv_protocol_is_ack(packet, ctx.ack_opcode, ctx.ack_sequence)) {
                        ctx.scan_offset = 0;

                        // Consume packet if found at the beginning of the buffer.
                        if ((void *) packet == buffer) {
                            omv_buffer_consume(&ctx.buffer, OMV_PROTOCOL_HEADER_SIZE);
                        }

                        if (packet->flags & OMV_PROTOCOL_FLAG_ACK) {
                            ctx.wait_for_ack = false;
                            ctx.ack_status = OMV_PROTOCOL_STATUS_SUCCESS;
                        } else {
                            ctx.ack_status = OMV_PROTOCOL_STATUS_FAILED;
                        }
                        return 0;
                    }
                }

                // No matching ACK found - stay in WAIT_ACK state
                return 0;
        }
    }

    return 0;
}

void omv_protocol_process(const omv_protocol_packet_t *packet) {
    ctx.stats.recv_packets++;

    switch (packet->opcode) {
        case OMV_PROTOCOL_OPCODE_PROTO_SYNC: {
            omv_protocol_reset();
            omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SUCCESS);
            ctx.sequence = 0;
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
            omv_protocol_caps_t *caps = (void *) packet->payload;

            // Validate only the protocol capability fields
            if (packet->length != sizeof(omv_protocol_caps_t) ||
                caps->max_payload < OMV_PROTOCOL_MIN_PAYLOAD_SIZE ||
                caps->max_payload > OMV_PROTOCOL_MAX_PAYLOAD_SIZE) {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
            } else {
                // ACK the updated caps first before changing them.
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SUCCESS);

                // Update only protocol capability fields, preserve transport config
                ctx.config.crc_enabled = caps->crc_enabled;
                ctx.config.seq_enabled = caps->seq_enabled;
                ctx.config.ack_enabled = caps->ack_enabled;
                ctx.config.event_enabled = caps->event_enabled;
                ctx.config.max_payload = caps->max_payload;
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_STATS: {
            omv_protocol_send_packet(packet->opcode, packet->channel, sizeof(ctx.stats), &ctx.stats, 0);
            break;
        }

        case OMV_PROTOCOL_OPCODE_PROTO_VERSION: {
            omv_protocol_version_t version = { 0 };

            // Protocol version
            version.protocol_version[0] = OMV_PROTOCOL_VERSION_MAJOR;
            version.protocol_version[1] = OMV_PROTOCOL_VERSION_MINOR;
            version.protocol_version[2] = OMV_PROTOCOL_VERSION_PATCH;

            // Bootloader version (read from flash at runtime if available)
            #if defined(OMV_FLASH_BOOT_ORIGIN)
            const omv_boot_header_t *boot_header =
                (const omv_boot_header_t *) (OMV_FLASH_BOOT_ORIGIN + OMV_BOOT_HEADER_OFFSET);
            if (boot_header->magic == OMV_BOOT_MAGIC_VALUE) {
                version.bootloader_version[0] = boot_header->major;
                version.bootloader_version[1] = boot_header->minor;
                version.bootloader_version[2] = boot_header->patch;
            } else
            #endif
            {
                version.bootloader_version[0] = OMV_BOOT_VERSION_MAJOR;
                version.bootloader_version[1] = OMV_BOOT_VERSION_MINOR;
                version.bootloader_version[2] = OMV_BOOT_VERSION_PATCH;
            }

            // Firmware version
            version.firmware_version[0] = OMV_FIRMWARE_VERSION_MAJOR;
            version.firmware_version[1] = OMV_FIRMWARE_VERSION_MINOR;
            version.firmware_version[2] = OMV_FIRMWARE_VERSION_PATCH;

            omv_protocol_send_packet(OMV_PROTOCOL_OPCODE_PROTO_VERSION,
                                     packet->channel, sizeof(version), &version, 0);
            break;
        }

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

        case OMV_PROTOCOL_OPCODE_SYS_INFO: {
            omv_protocol_sys_info_t sysinfo = { 0 };

            // Hardware identification
            sysinfo.cpu_id = SCB->CPUID;

            // USB VID/PID
            #if defined(OMV_USB_VID) && defined(OMV_USB_PID)
            sysinfo.usb_id = ((uint32_t) OMV_USB_VID << 16) | OMV_USB_PID;
            #endif

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
            for (size_t i = 0; chip_count < max_chip_ids && i < OMV_CSI_MAX_DEVICES; i++) {
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
            sysinfo.frame_buffer_size_kb = framebuffer_get(FB_MAINFB_ID)->raw_size / 1024;
            sysinfo.stream_buffer_size_kb = framebuffer_get(FB_STREAM_ID)->raw_size / 1024;

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

            if (!omv_protocol_check_timeout(ctx.last_lock_ms, ctx.config.lock_intval_ms)) {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_BUSY);
            } else if (channel && channel->lock && channel->lock(channel) == 0) {
                ctx.last_lock_ms = OMV_PROTOCOL_TICKS_MS();
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SUCCESS);
            } else {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_BUSY);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_UNLOCK: {
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->unlock && channel->unlock(channel) == 0) {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SUCCESS);
            } else {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_BUSY);
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
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
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
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_READ: {
            omv_protocol_channel_io_t *request = (void *) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (!request->length || !channel || !(channel->read || channel->readp)) {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
            } else if (channel->readp) {
                const void *data = channel->readp(channel, request->offset, request->length);
                if (!data) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_FAILED);
                } else {
                    omv_protocol_send_packet(packet->opcode, packet->channel, request->length, data, 0);
                }
            } else {
                // NOTE: We can't use the packet pointer or its payload after calling send_packet.
                uint32_t length = request->length;
                uint32_t offset = request->offset;
                uint8_t buffer[OMV_MIN(512, ctx.config.max_payload)];

                while (length > 0) {
                    size_t size_rq = OMV_MIN(length, sizeof(buffer));
                    int32_t size_rd = channel->read(channel, offset, size_rq, buffer);

                    if (size_rd <= 0) {
                        omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_FAILED);
                        break;
                    }

                    length -= size_rd;
                    offset += size_rd;
                    uint8_t flags = (length == 0) ? 0 : OMV_PROTOCOL_FLAG_FRAGMENT;
                    omv_protocol_send_packet(packet->opcode, packet->channel, size_rd, buffer, flags);
                }
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_WRITE: {
            omv_protocol_channel_io_t *request = (void *) packet->payload;
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->write && request->length) {
                if (channel->write(channel, request->offset, request->length, request->payload)) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SUCCESS);
                } else {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_FAILED);
                }
            } else {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        case OMV_PROTOCOL_OPCODE_CHANNEL_IOCTL: {
            omv_protocol_channel_ioctl_t *ioctl = (void *) packet->payload;
            size_t ioctl_len = packet->length - offsetof(omv_protocol_channel_ioctl_t, payload);
            uint8_t *ioctl_arg = (ioctl_len) ? ioctl->payload : NULL;
            const omv_protocol_channel_t *channel = omv_protocol_find_channel(packet->channel);

            if (channel && channel->ioctl) {
                // Validate argument size for static channels only
                if (!OMV_PROTOCOL_CHANNEL_FLAG_GET(channel, DYNAMIC) && // TODO
                    !omv_protocol_ioctl_check(packet->channel, ioctl->request, ioctl_len)) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
                } else if (channel->ioctl(channel, ioctl->request, ioctl_len, ioctl_arg)) {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_FAILED);
                } else {
                    omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_SUCCESS);
                }
            } else {
                omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
            }
            break;
        }

        default:
            omv_protocol_send_status(packet, OMV_PROTOCOL_STATUS_INVALID);
            break;
    }
}

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <inttypes.h>
#include <libc.h>
#include <protocol.h>
#include <da.h>
#include <debug.h>

#define DA_LOG_BUF_SIZE 512

com_channel_struct* g_com_channel = NULL;

static char da_log_buf[DA_LOG_BUF_SIZE];
static int  da_log_pos = 0;

void *(*malloc)(size_t size) = (const void*)0x22222222;
void (*free)(void *ptr) = (const void*)0x33333333;
void *(*mmc_get_card)(int card_id) = (const void*)0x44444444;

static void da_log_flush(void)
{
    if (da_log_pos == 0 || g_com_channel == NULL)
        return;

    g_com_channel->log_to_pc((const uint8_t *)da_log_buf, (uint32_t)da_log_pos);
    da_log_pos = 0;
}

static void da_log_putc(int ch, void *ctx)
{
    (void)ctx;

    if (da_log_pos >= DA_LOG_BUF_SIZE)
        da_log_flush();

    da_log_buf[da_log_pos++] = (char)ch;

    if (ch == '\n')
        da_log_flush();
}

void da_log_register(com_channel_struct *channel)
{
    if (g_com_channel != NULL)
        return;

    g_com_channel = channel;
    printf_register_cb(da_log_putc);
}


int download_data(com_channel_struct* channel, uint8_t** dst, uint64_t size, const char* desc) {
    uint32_t packet_length = g_da_ctx.write_packet_size;
    uint64_t total_received = 0;
    uint64_t xfered = 0;
    uint32_t ack = 0;
    uint32_t ack_length = 4;
    uint32_t checksum = 0;

    printf("Starting download of 0x%" PRIx32 " bytes for %s\n", (uint32_t)size, desc);

    if (packet_length == 0)
        packet_length = 0x8000;

    if (*dst == NULL) {
        *dst = (uint8_t*)malloc(size + 4);
        if (*dst == NULL) {
            printf("Failed to allocated %u bytes for %s\n", size, desc);
            return STATUS_MALLOC_FAILED;
        }
        memset(*dst, 0, size + 4);
    }

    while (total_received < size) {
        ack_length = 4;
        channel->read((uint8_t*)&ack, &ack_length);
        if (ack != 0) {
            printf("Host cancelled %s data transfer!\n", desc);
            return STATUS_DOWNLOAD_ACK_NOT_OK;
        }

        ack_length = 4;
        channel->read((uint8_t*)&checksum, &ack_length);
        // TODO: Verify the checksum

        uint32_t to_receive = size - total_received;
        to_receive = to_receive > packet_length ? packet_length : to_receive;
        int status = channel->read((uint8_t*)(*dst + total_received), &to_receive);
        if (status != 0) {
            printf("Failed to read %s data packet!\n", desc);
            return status;
        }

        channel->write((uint8_t*)&ack, 4);

        total_received += to_receive;
        xfered += to_receive;
    }

    // STATUS_OK
    channel->write((uint8_t*)&ack, 4);

    printf("Download complete: received 0x%" PRIx32 " bytes for %s\n", (uint32_t)xfered, desc);

    return ack;
}

int upload_data(com_channel_struct* channel, const uint8_t* src, uint64_t size, const char* desc) {
    uint32_t packet_length = g_da_ctx.read_packet_size;
    if (packet_length == 0) packet_length = size;
    uint64_t total_sent = 0;
    uint32_t to_read = 4;

    printf("Starting upload of 0x%" PRIx32 " bytes for %s\n", (uint32_t)size, desc);

    while (total_sent < size) {
        uint32_t to_send = (size - total_sent) > packet_length ? packet_length : (size - total_sent);
        int status = channel->write((uint8_t*)(src + total_sent), to_send);
        if (status != 0) {
            printf("Failed to write %s data packet!\n", desc);
            return status;
        }

        to_read = 4;
        channel->read((uint8_t*)&status, &to_read);
        if (status != 0) {
            printf("Host cancelled %s data transfer!\n", desc);
            return STATUS_UPLOAD_ACK_NOT_OK;
        }

        channel->write((uint8_t*)&status, 4);

        total_sent += to_send;
    }

    printf("Upload complete: sent 0x%" PRIx32 " bytes for %s\n", (uint32_t)total_sent, desc);

    return 0;
}

int download_data_stream(com_channel_struct* channel, uint64_t size, uint32_t chunk_size, data_stream_cb cb, void *ctx, const char* desc) {
    uint64_t total_received = 0;
    uint32_t ack = 0;
    uint32_t ack_length = 4;
    uint32_t checksum = 0;
    int status = 0;

    if (chunk_size == 0) chunk_size = g_da_ctx.write_packet_size;
    if (chunk_size == 0) chunk_size = 0x10000;
    if (chunk_size > size) chunk_size = size;

    uint8_t *buf = malloc(chunk_size);
    if (!buf) return STATUS_MALLOC_FAILED;

    printf("Starting stream download of 0x%" PRIx32 " bytes for %s\n", (uint32_t)size, desc);

    while (total_received < size) {
        ack_length = 4;
        channel->read((uint8_t*)&ack, &ack_length);
        if (ack != 0) {
            printf("Host cancelled %s data transfer!\n", desc);
            status = STATUS_DOWNLOAD_ACK_NOT_OK;
            goto out;
        }

        ack_length = 4;
        channel->read((uint8_t*)&checksum, &ack_length);
        uint32_t to_receive = (size - total_received) > chunk_size ? chunk_size : (size - total_received);
        uint32_t chunk_received = 0;

        while (chunk_received < to_receive) {
            uint32_t read_len = to_receive - chunk_received;
            status = channel->read(buf, &read_len);
            if (status != 0) {
                printf("Failed to read %s data packet!\n", desc);
                goto out;
            }

            if (cb) {
                status = cb(total_received + chunk_received, buf, read_len, ctx);
                if (status != 0) goto out;
            }

            chunk_received += read_len;
        }

        channel->write((uint8_t*)&ack, 4);

        total_received += to_receive;
    }

    // STATUS_OK
    channel->write((uint8_t*)&ack, 4);

out:
    free(buf);
    return status;
}

int upload_data_stream(com_channel_struct* channel, uint64_t size, uint32_t chunk_size, data_stream_cb cb, void *ctx, const char* desc) {
    uint64_t total_sent = 0;
    uint32_t to_read = 4;
    int status = 0;

    if (chunk_size == 0) chunk_size = g_da_ctx.read_packet_size;
    if (chunk_size == 0) chunk_size = 0x10000;
    if (chunk_size > size) chunk_size = size;

    uint8_t *buf = malloc(chunk_size);
    if (!buf) return STATUS_MALLOC_FAILED;

    printf("Starting stream upload of 0x%" PRIx32 " bytes for %s\n", (uint32_t)size, desc);

    while (total_sent < size) {
        uint32_t to_send = (size - total_sent) > chunk_size ? chunk_size : (size - total_sent);

        if (cb) {
            status = cb(total_sent, buf, to_send, ctx);
            if (status != 0) goto out;
        }

        status = channel->write(buf, to_send);
        if (status != 0) {
            printf("Failed to write %s data packet!\n", desc);
            goto out;
        }

        uint32_t ack = 0;
        to_read = 4;
        status = channel->read((uint8_t*)&ack, &to_read);
        if (status != 0 || ack != 0) {
            printf("Host cancelled %s data transfer!\n", desc);
            status = STATUS_UPLOAD_ACK_NOT_OK;
            goto out;
        }
        channel->write((uint8_t*)&ack, 4);

        total_sent += to_send;
    }

out:
    free(buf);
    return status;
}

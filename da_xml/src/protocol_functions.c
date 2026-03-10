/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <protocol_functions.h>
#include <nanoprintf.h>
#include <debug.h>
#include <libc.h>

#define XML_CMD_BUFF_LEN 0x80000
#define CMD_RESULT_BUFF_LEN 64

void (*register_major_command)(const char *, const char *, HHANDLE) = (const void*)0x22222222;

void *(*malloc)(size_t size) = (const void*)0x55555555;
void (*free)(void *ptr) = (const void*)0x66666666;

char *(*mxmlGetNodeText)(void*, const char*) = (void*)0x77777777;
void *(*mxmlLoadString)(void*, const char*, void*) = (void*)0x88888888;

int download(struct com_channel_struct *channel, const char *filename, char **data_buf, uint32_t *data_len, const char *desc)
{
    printf("Download host file: %s\n", filename);

    if (!channel || !data_buf || !data_len) {
        printf("Invalid arguments to download\n");
        return STATUS_ERR;
    }

    uint32_t packet_length = 0x200000;
    char xml[XML_CMD_BUFF_LEN] = {0};
    const char *cstr_cmd = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                           "<host>"
                           "<version>1.0</version>"
                           "<command>CMD:DOWNLOAD-FILE</command>"
                           "<arg><checksum>CHK_NO</checksum>"
                           "<info>%s</info>"
                           "<source_file>%s</source_file>"
                           "<packet_length>0x%x</packet_length></arg>"
                           "</host>";

    int xml_len = npf_snprintf(xml, sizeof(xml), cstr_cmd, desc, filename, packet_length);

    // + 1 for accounting of \0
    if (channel->write((uint8_t *)xml, (uint32_t)xml_len + 1) != 0) {
        printf("Failed to send download command\n");
        return STATUS_ERR;
    }

    char result[CMD_RESULT_BUFF_LEN] = {0};
    uint32_t len = sizeof(result);
    if (channel->read((uint8_t *)result, &len) != 0) {
        printf("Failed to read first response\n");
        return STATUS_ERR;
    }

    char *vec[2] = {0};
    split(result, vec, 2, '@');
    if (strncmp(vec[0], "OK", 2) != 0) {
        printf("Host reported error: %s\n", result);
        return STATUS_ERR;
    }

    char buf_total_length[CMD_RESULT_BUFF_LEN] = {0};
    len = sizeof(buf_total_length);
    if (channel->read((uint8_t *)buf_total_length, &len) != 0) {
        printf("Failed to read total length\n");
        channel->write((uint8_t *)"ERR", 4);
        return STATUS_ERR;
    }

    split(buf_total_length, vec, 2, '@');
    if (strncmp(vec[0], "OK", 2) != 0) {
        printf("Host reported error on length: %s\n", buf_total_length);
        channel->write((uint8_t *)"ERR", 4);
        return STATUS_ERR;
    }

    uint32_t total_length = atoui(vec[1]);
    printf("Download: total_length=0x%x\n", total_length);

    if (*data_buf != NULL) {
        if (*data_len <= total_length) {
            printf("Buffer too small: have %u, need %u\n", *data_len, total_length);
            channel->write((uint8_t *)"ERR", 4);
            return STATUS_ERR;
        }
        *data_len = total_length;
    } else {
        *data_len = total_length;
        *data_buf = (char *)malloc(total_length + 4);
        if (*data_buf == NULL) {
            printf("Failed to allocate %u bytes for %s\n", total_length, desc);
            channel->write((uint8_t *)"ERR", 4);
            return STATUS_ERR;
        }
        memset(*data_buf, 0, total_length + 4);
    }

    channel->write((uint8_t *)"OK", 3);

    uint32_t xfered = 0;
    while (xfered < total_length) {
        len = sizeof(result);
        if (channel->read((uint8_t *)result, &len) != 0) {
            printf("Failed to read chunk signal\n");
            channel->write((uint8_t *)"ERR", 4);
            return STATUS_ERR;
        }

        split(result, vec, 2, '@');
        if (strncmp(vec[0], "OK", 2) != 0) {
            printf("Host cancelled or errored: %s\n", result);
            channel->write((uint8_t *)"ERR", 4);
            return STATUS_ERR;
        }

        channel->write((uint8_t *)"OK", 3);

        uint32_t chunk = total_length - xfered;
        if (chunk > packet_length)
            chunk = packet_length;

        if (channel->read((uint8_t *)*data_buf + xfered, &chunk) != 0) {
            printf("Failed to read data chunk at offset 0x%x\n", xfered);
            channel->write((uint8_t *)"ERR", 4);
            return STATUS_ERR;
        }
        xfered += chunk;

        channel->write((uint8_t *)"OK", 3);
    }

    (*data_buf)[total_length] = 0;
    printf("Downloaded %u bytes for %s\n", xfered, desc);
    return STATUS_OK;
}

int upload(struct com_channel_struct *channel, const char *filename, const char *buf, uint32_t size, const char *desc)
{
    printf("Upload host file: %s\n", filename);

    if (!channel || !filename) {
        printf("Invalid arguments to upload\n");
        return STATUS_ERR;
    }

    uint32_t packet_length = 0x200000;
    char xml[XML_CMD_BUFF_LEN] = {0};
    const char *cstr_cmd = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                           "<host>"
                           "<version>1.0</version>"
                           "<command>CMD:UPLOAD-FILE</command>"
                           "<arg><checksum>CHK_NO</checksum>"
                           "<info>%s</info>"
                           "<target_file>%s</target_file>"
                           "<packet_length>0x%x</packet_length></arg>"
                           "</host>";

    int xml_len = npf_snprintf(xml, sizeof(xml), cstr_cmd, desc, filename, packet_length);

    if (channel->write((uint8_t *)xml, (uint32_t)xml_len + 1) != 0) {
        printf("Failed to send upload command\n");
        return STATUS_ERR;
    }

    char result[CMD_RESULT_BUFF_LEN] = {0};
    uint32_t len = sizeof(result);
    if (channel->read((uint8_t *)result, &len) != 0) {
        printf("Failed to read response\n");
        return STATUS_ERR;
    }

    char *vec[2] = {0};
    split(result, vec, 2, '@');
    if (strncmp(vec[0], "OK", 2) != 0) {
        printf("Host reported error: %s\n", result);
        return STATUS_ERR;
    }

    npf_snprintf(result, sizeof(result), "OK@0x%" PRIx32, size);
    channel->write((uint8_t *)result, (uint32_t)strlen(result) + 1);

    len = sizeof(result);
    if (channel->read((uint8_t *)result, &len) != 0) {
        printf("Failed to read size ack\n");
        return STATUS_ERR;
    }

    uint32_t xfered = 0;
    uint32_t left = size;
    while (left > 0) {
        channel->write((uint8_t *)"OK", 3);

        len = sizeof(result);
        if (channel->read((uint8_t *)result, &len) != 0) {
            printf("Failed to read chunk ack\n");
            return STATUS_ERR;
        }

        uint32_t todo = left > packet_length ? packet_length : left;
        channel->write((uint8_t *)(buf + xfered), todo);
        xfered += todo;
        left -= todo;

        len = sizeof(result);
        if (channel->read((uint8_t *)result, &len) != 0) {
            printf("Failed to read chunk status\n");
            return STATUS_ERR;
        }

        split(result, vec, 2, '@');
        if (strncmp(vec[0], "OK", 2) != 0) {
            printf("Host error during upload: %s\n", result);
            return STATUS_ERR;
        }
    }

    printf("Uploaded %u bytes for %s\n", xfered, desc);
    return STATUS_OK;
}

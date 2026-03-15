/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_XML_PROTOCOL_FUNCTIONS_H
#define DA_XML_PROTOCOL_FUNCTIONS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <libc.h>
#include <debug.h>
#include <stream.h>
#include <xml.h>

struct com_channel_struct {
  int (*read)(uint8_t *buffer, uint32_t *length);
  int (*write)(uint8_t *buffer, uint32_t length);
  int (*log_to_pc)(const uint8_t *buffer, uint32_t length);
  int (*log_to_uart)(const uint8_t *buffer, uint32_t length);
};

#define STATUS_OK (0x00000000)
#define STATUS_ERR (0xC0010001)

typedef int (*HHANDLE)(struct com_channel_struct* /* channel */, const char* /* xml */);

// Protocol functions
extern void (*register_major_command)(const char *, const char *, HHANDLE);
int download(struct com_channel_struct *channel, const char *filename, char **data_buf, uint32_t *data_len, const char *desc);
int upload(struct com_channel_struct *channel, const char *filename, const char *data_buf, uint32_t data_len, const char *desc);
int download_stream(struct com_channel_struct *channel, const char *filename, uint32_t chunk_size, data_stream_cb cb, void *ctx, const char *desc);
int upload_stream(struct com_channel_struct *channel, const char *filename, uint32_t size, uint32_t chunk_size, data_stream_cb cb, void *ctx, const char *desc);

// DA logging - registers the channel globally and hooks up printf -> log_to_pc
void da_log_register(struct com_channel_struct *channel);

// Memory
extern void *(*malloc)(size_t size);
extern void (*free)(void *ptr);

// Storage
extern void *(*mmc_get_card)(int card_id);

// XML
extern char *(*mxmlGetNodeText)(void* /* tree */, const char* /* path */);
extern void *(*mxmlLoadString)(void*, const char*, void* /* callback */); // OPAQUE = 2

extern struct com_channel_struct *global_channel;

#endif //DA_XML_PROTOCOL_FUNCTIONS_H

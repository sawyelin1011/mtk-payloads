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
#include <search.h>
#include <xml.h>

struct com_channel_struct {
  int (*read)(uint8_t *buffer, uint32_t *length);
  int (*write)(uint8_t *buffer, uint32_t length);
  int (*log_to_pc)(const uint8_t *buffer, uint32_t length);
  int (*log_to_uart)(const uint8_t *buffer, uint32_t length);
};

#define STATUS_OK (0x00000000)
#define STATUS_ERR (0xC0010001)

#define MAX_ALT_PATTERNS 4

typedef struct {
    const uint32_t *pattern;
    size_t pattern_len;
    const uint32_t *mask;
} pattern_variant_t;

typedef struct {
    pattern_variant_t variants[MAX_ALT_PATTERNS];
    size_t num_variants;
    void **func_ptr;
    const char *func_name;
} function_pattern_t;

#define PATTERN_VARIANT(name) \
    { name##_PATTERN, name##_PLEN, name##_MASK }

#define FUNC_SINGLE(name, ptr, fname) \
    { \
        .variants = { PATTERN_VARIANT(name) }, \
        .num_variants = 1, \
        .func_ptr = (void**)(ptr), \
        .func_name = fname \
    }

#define FUNC_DUAL(name1, name2, ptr, fname) \
    { \
        . variants = { PATTERN_VARIANT(name1), PATTERN_VARIANT(name2) }, \
        .num_variants = 2, \
        .func_ptr = (void**)(ptr), \
        .func_name = fname \
    }

#define FUNC_TRIPLE(name1, name2, name3, ptr, fname) \
    { \
        .variants = { PATTERN_VARIANT(name1), PATTERN_VARIANT(name2), PATTERN_VARIANT(name3) }, \
        .num_variants = 3, \
        .func_ptr = (void**)(ptr), \
        .func_name = fname \
    }

typedef int (*HHANDLE)(struct com_channel_struct* /* channel */, const char* /* xml */);

// Protocol functions
extern void (*register_major_command)(const char *, const char *, HHANDLE);
int download(struct com_channel_struct *channel, const char *filename, char **data_buf, uint32_t *data_len, const char *desc);
int upload(struct com_channel_struct *channel, const char *filename, const char *data_buf, uint32_t data_len, const char *desc);

// Memory
extern void *(*malloc)(size_t size);
extern void (*free)(void *ptr);

// UART logs :D
extern int (*dprintf)(const char *fmt, ...);

// XML
extern char *(*mxmlGetNodeText)(void* /* tree */, const char* /* path */);
extern void *(*mxmlLoadString)(void*, const char*, void* /* callback */); // OPAQUE = 2

#endif //DA_XML_PROTOCOL_FUNCTIONS_H

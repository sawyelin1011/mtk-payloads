/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef int (*data_stream_cb)(uint64_t offset, uint8_t *data, uint32_t length, void *ctx);

typedef struct {
    data_stream_cb pull; /* Callback to pull data from the source */
    data_stream_cb push; /* Callback to push data to the destination */
    void *ctx;           /* Context to be passed to the callbacks */
} data_stream_t;

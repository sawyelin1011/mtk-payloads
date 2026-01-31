/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <list.h>

#ifdef __aarch64__
#include <mutex.h>
#endif

struct heap {
    void *base;
    size_t len;
#ifdef __aarch64__
    size_t remaining;
    size_t low_watermark;
    struct mutex lock;
#endif
    struct list_node free_list;
};

struct free_heap_chunk {
    struct list_node node;
    size_t len;
};

/*
 * ARM32: [magic][ptr][size]
 * ARM64: [ptr][size]
 */
struct alloc_struct_begin {
#ifndef __aarch64__
    unsigned int magic;
#endif
    void *ptr;
    size_t size;
};

#ifdef __aarch64__
#define PTR_ALIGN_MASK  0x7   /* 8-byte alignment */
#else
#define HEAP_MAGIC      0x68656170  /* 'heap' */
#define PTR_ALIGN_MASK  0x3
#endif

/*
 * DPC struct layout:
 *
 * ARM32:
 *   +0x00: prev (fake, from list_node)
 *   +0x04: next (fake, from list_node)
 *   +0x08: key
 *   +0x0c: cb
 *   +0x10: arg
 *
 * ARM64:
 *   +0x00: prev (fake, from list_node)
 *   +0x08: next (fake, from list_node)
 *   +0x10: key
 *   +0x18: cb
 *   +0x20: arg
 *
 * The corrupted node pointer points to the "prev" field.
 * We want to clear starting from "key" onwards.
 */
#ifdef __aarch64__
#define DPC_KEY_OFFSET 0x10
#define DPC_CLEAR_SIZE 0x20
#else
#define DPC_KEY_OFFSET 0x08
#define DPC_CLEAR_SIZE 0x10
#endif

typedef void (*cmd_dpc_cb)(void *arg);

struct cmd_dpc_t {
  cmd_dpc_cb cb;
  void *arg;
};

struct cmd_dpc_t *get_cmd_dpc();
extern struct cmd_dpc_t dpc;

void heap_dump(uintptr_t addr);
void heap_fix(uintptr_t addr);

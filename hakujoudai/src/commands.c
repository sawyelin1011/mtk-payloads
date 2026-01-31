/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#include <stdint.h>
#include <libc.h>
#include <heap.h>
#include <commands.h>
#include <debug.h>

#define STATUS_OK   0x00000000
#define STATUS_ERR  0xC0010001

static int (*const volatile download)(struct com_channel_struct *, const char *, char **, uint32_t *, const char *) = (void *)0x44444444;
static void (*const volatile da_free)(void *) = (void *)0x55555555;
static char *(*const volatile mxmlGetNodeText)(void *, const char *) = (void *)0x66666666;
static void *(*const volatile mxmlLoadString)(void *, const char *, void *) = (void *)0x77777777;

static int cb_opaque(void *unused)
{
    (void)unused;
    return 2;
}

int cmd_boot_to(struct com_channel_struct *channel, const char *xml)
{
    (void)xml;
    volatile uintptr_t ext_addr = 0x68000000;
    char *buf = (char *)ext_addr;
    uint32_t len = 0x1000000;

    int status = download(channel, "ext", &buf, &len, "ext");
    if (status != STATUS_OK)
        return status;

    get_cmd_dpc()->cb = (cmd_dpc_cb)ext_addr;
    get_cmd_dpc()->arg = &status;

    return status;
}

int cmd_patch_mem(struct com_channel_struct *channel, const char *xml)
{
    void *tree = mxmlLoadString(NULL, xml, cb_opaque);
    if (!tree)
        return STATUS_ERR;

    uintptr_t addr = (uintptr_t)atoull(mxmlGetNodeText(tree, "da/arg/address"));
    uint32_t len = (uint32_t)atoui(mxmlGetNodeText(tree, "da/arg/length")) + 4; // + 4 or download fails on *pdata_len <= total_length

    int status = download(channel, "mempatch.bin", (void *)&addr, &len, "memory patch");

    da_free(tree);
    return status;
}

int cmd_call_function(struct com_channel_struct *channel, const char *xml)
{
    (void)channel;
    void *tree = mxmlLoadString(NULL, xml, cb_opaque);
    if (!tree)
        return STATUS_ERR;

    uintptr_t addr = (uintptr_t)atoull(mxmlGetNodeText(tree, "da/arg/address"));
    get_cmd_dpc()->cb = (cmd_dpc_cb)addr;

    da_free(tree);
    return STATUS_OK;
}

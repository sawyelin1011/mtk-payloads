/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy, R0rt1z2
 */

#include <stdint.h>
#include <inttypes.h>
#include <libc.h>
#include <heap.h>
#include <commands.h>
#include <debug.h>

#define STATUS_OK   0x00000000
#define STATUS_ERR  0xC0010001

#define MXML_TYPE_ELEMENT  1
#define MXML_TYPE_TEXT     2
#define MXML_TYPE_OPAQUE   3

static int (*const volatile download)(struct com_channel_struct *, const char *, char **, uint32_t *, const char *) = (void *)0x44444444;
static void (*const volatile da_free)(void *) = (void *)0x55555555;
static void *(*const volatile mxmlGetNodeText)(void *, const char *) = (void *)0x66666666;
static void *(*const volatile mxmlLoadString)(void *, const char *, void *) = (void *)0x77777777;

static int cb_opaque(void *unused)
{
    (void)unused;
    return 2;
}

/*
 * This is required because MediaTek managed to break ABI compatibility
 * even in their own mxml implementation across DA versions. Some return
 * strings directly, others return node pointers. Classic MediaTek...
 */
static const char *get_node_text(void *tree, const char *path)
{
    void *result = mxmlGetNodeText(tree, path);
    if (!result)
        return NULL;

    uintptr_t *n = (uintptr_t *)result;

    /* Text/opaque node: type 2|3, no children, has value */
    if ((n[0] == MXML_TYPE_TEXT || n[0] == MXML_TYPE_OPAQUE) &&
        n[4] == 0 && n[5] == 0 && n[6] != 0)
        return (const char *)n[6];

    /* Element node with text child */
    if (n[0] == MXML_TYPE_ELEMENT && n[4] != 0) {
        uintptr_t *c = (uintptr_t *)n[4];
        if ((c[0] == MXML_TYPE_TEXT || c[0] == MXML_TYPE_OPAQUE) &&
            c[4] == 0 && c[5] == 0 && c[6] != 0)
            return (const char *)c[6];
    }

    /* Assume direct string */
    return (const char *)result;
}

int cmd_boot_to(struct com_channel_struct *channel, const char *xml)
{
    (void)xml;
    volatile uintptr_t ext_addr = 0x68000000;
    char *buf = (char *)ext_addr;
    uint32_t len = 0x1000000;

    printf("%s: loading extensions to 0x%" PRIxPTR " (max 0x%" PRIx32 " bytes)\n",
           __func__, (uintptr_t)ext_addr, len);

    int status = download(channel, "ext", &buf, &len, "ext");
    if (status != STATUS_OK) {
        printf("%s: download failed: 0x%" PRIx32 "\n", __func__, status);
        return status;
    }

    printf("%s: scheduling call to 0x%" PRIxPTR "\n", __func__, (uintptr_t)ext_addr);

    get_cmd_dpc()->cb = (cmd_dpc_cb)ext_addr;
    get_cmd_dpc()->arg = &status;

    return status;
}

int cmd_patch_mem(struct com_channel_struct *channel, const char *xml)
{
    void *tree = mxmlLoadString(NULL, xml, cb_opaque);
    if (!tree) {
        printf("%s: failed to parse XML\n", __func__);
        return STATUS_ERR;
    }

    const char *addr_str = get_node_text(tree, "da/arg/address");
    const char *len_str  = get_node_text(tree, "da/arg/length");

    if (!addr_str || !len_str) {
        printf("%s: missing address/length\n", __func__);
        da_free(tree);
        return STATUS_ERR;
    }

    char *dst = (char *)(uintptr_t)atoull(addr_str);
    printf("%s: patching %" PRIu32 " bytes at 0x%" PRIxPTR "\n",
           __func__, atoui(len_str), (uintptr_t)dst);

    /* + 4 or download fails on '*pdata_len <= total_length' */
    uint32_t size = atoui(len_str) + 4;

    int status = download(channel, "mempatch.bin", &dst, &size, "memory patch");
    if (status != STATUS_OK) {
        printf("%s: download failed: 0x%" PRIx32 "\n", __func__, status);
        da_free(tree);
        return status;
    }

    da_free(tree);
    return STATUS_OK;
}

int cmd_call_function(struct com_channel_struct *channel, const char *xml)
{
    (void)channel;

    void *tree = mxmlLoadString(NULL, xml, cb_opaque);
    if (!tree) {
        printf("%s: failed to parse XML\n", __func__);
        return STATUS_ERR;
    }

    const char *addr_str = get_node_text(tree, "da/arg/address");
    if (!addr_str) {
        printf("%s: missing address argument\n", __func__);
        da_free(tree);
        return STATUS_ERR;
    }

    uintptr_t addr = (uintptr_t)atoull(addr_str);
    printf("%s: scheduling call to 0x%" PRIxPTR "\n", __func__, addr);

    get_cmd_dpc()->cb = (cmd_dpc_cb)addr;

    da_free(tree);
    return STATUS_OK;
}

/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stddef.h>
#include <protocol_functions.h>
#include <commands.h>
#include <sej.h>
#include <nanoprintf.h>

int cb_opaque(void*) {
    return 2;
}

#define CB_OPAQUE cb_opaque

int cmd_ack(struct com_channel_struct *channel, const char*) {
    int status = STATUS_OK;
    const char *target_file = "ack.xml";
    char ack_xml[512];

    int len = npf_snprintf(ack_xml, sizeof(ack_xml),
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ack>"
        "<status>OK</status>"
        "<register_major_command>0x%08x</register_major_command>"
        "<download>0x%08x</download>"
        "<upload>0x%08x</upload>"
        "<malloc>0x%08x</malloc>"
        "<free>0x%08x</free>"
        "<get_node_text>0x%08x</get_node_text>"
        "<mxmlLoadString>0x%08x</mxmlLoadString>"
        "</ack>",
        (unsigned int)(uintptr_t)register_major_command,
        (unsigned int)(uintptr_t)download,
        (unsigned int)(uintptr_t)upload,
        (unsigned int)(uintptr_t)malloc,
        (unsigned int)(uintptr_t)free,
        (unsigned int)(uintptr_t)get_node_text,
        (unsigned int)(uintptr_t)mxmlLoadString
    );

    status = upload(channel, target_file, ack_xml, (uint32_t)len, "ACK");

    printf("DA Extension ACK sent\n");

    return status;
}

int cmd_readmem(struct com_channel_struct *channel, const char* xml) {
    int status = STATUS_OK;
    const char *target_file = "readmem.bin";
    void *tree;
    uintptr_t address;
    u32 length;

    MXML_LOAD(tree, xml, CB_OPAQUE, "da/arg/address", "da/arg/length", NULL);

    address = atoull(get_node_text(tree, "da/arg/address"));
    length = atoull(get_node_text(tree, "da/arg/length"));

    printf("ReadMem: address=0x%08lx length=0x%08x\n", address, length);

    status = upload(channel, target_file, (const char*)address, length, "memory read");

    MXMLDELETE(tree);
    return status;
}

int cmd_writemem(struct com_channel_struct *channel, const char* xml) {
    int status = STATUS_OK;
    const char *source_file = "writemem.bin";
    void *tree;
    char *address;
    uint32_t length;

    MXML_LOAD(tree, xml, CB_OPAQUE, "da/arg/address", "da/arg/length", NULL);

    address = (char *)(uintptr_t)atoull(get_node_text(tree, "da/arg/address"));
    length = atoull(get_node_text(tree, "da/arg/length"));

    printf("WriteMem: address=0x%08lx length=0x%08x\n", (uintptr_t)address, length);

    status = download(channel, source_file, &address, &length, "memory write");

    MXMLDELETE(tree);
    return status;
}

int cmd_sej_aes(struct com_channel_struct *channel, const char* xml) {
    #define AES_MAX_LEN 4096
    int status = STATUS_OK;
    const char *source_file = "sej_aes.bin";
    uint32_t data_length = 0;
    void* data_buf = NULL;
    void *tree;
    bool anti_clone;
    bool encrypt;
    bool legacy;

    MXML_LOAD(tree, xml, CB_OPAQUE, "da/arg/encrypt", "da/arg/legacy", "da/arg/ac", NULL);

    encrypt    = (strcmp(get_node_text(tree, "da/arg/encrypt"), "yes") == 0);
    legacy     = (strcmp(get_node_text(tree, "da/arg/legacy"), "yes") == 0);
    anti_clone = (strcmp(get_node_text(tree, "da/arg/ac"),     "yes") == 0);

    printf("SEJ AES: encrypt=%d legacy=%d anti_clone=%d\n", encrypt, legacy, anti_clone);

    status = download(channel, source_file, (char**)&data_buf, &data_length, "SEJ AES data");

    printf("SEJ AES: download status=%d data_buf=%p data_length=0x%08x\n",
        status, data_buf, data_length);

    if (status != STATUS_OK) {
        printf("SEJ AES: download failed\n");
        goto free;
    }

    if (data_length > AES_MAX_LEN) {
        printf("SEJ AES: rejecting data_length=0x%08x > AES_MAX_LEN=0x%08x\n",
            data_length, AES_MAX_LEN);
        status = STATUS_ERR;
        goto end;
    }

    if (encrypt)
        sp_sej_enc(data_buf, data_buf, data_length, anti_clone, legacy);
    else
        sp_sej_dec(data_buf, data_buf, data_length, anti_clone, legacy);

    status = upload(channel, source_file, (const char*)data_buf, data_length, "SEJ AES result");

    printf("SEJ AES: upload status=%d\n", status);

free:
    if (data_buf)
        free(data_buf);
end:
    MXMLDELETE(tree);
    printf("SEJ AES: done, status=%d\n", status);
    return status;
}

int cmd_set_sej_base(struct com_channel_struct*, const char* xml) {
    int status = STATUS_OK;
    void *tree;
    uintptr_t sej_base;

    MXML_LOAD(tree, xml, CB_OPAQUE, "da/arg/sej_base", NULL);

    sej_base = atoull(get_node_text(tree, "da/arg/sej_base"));

    printf("Set SEJ Base: 0x%08lx\n", sej_base);
    set_sej_base(sej_base);

    MXMLDELETE(tree);
    return status;
}

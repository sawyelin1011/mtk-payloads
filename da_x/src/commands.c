/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <libc.h>
#include <debug.h>
#include <crypto/tzcc.h>
#include <crypto/key_derive.h>
#include <security/rpmb.h>
#include <storage/mmc/rpmb_mmc.h>
#include <sej.h>
#include <hal.h>
#include <da.h>
#include <protocol.h>

volatile da_ctx_t g_da_ctx;

int cmd_ack(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    uint32_t ack=STATUS_OK;

    printf("Status OK!\n");
    return channel->write((uint8_t *)&ack,4);
}

int cmd_setup_da_ctx(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    da_ctx_t da_ctx;
    uint32_t size = sizeof(da_ctx);
    int status = 0;

    status = channel->read((uint8_t*)&da_ctx, &size);
    if (status != 0) {
        printf("Failed to read DA context!\n");
        printf("Some functionality may not work correctly.\n");
        return status;
    }

    g_da_ctx = da_ctx;

    if (g_da_ctx.usb_log) {
        printf("Enabling USB logging\n");
        da_log_register(channel);
    }

    set_sej_base(g_da_ctx.sej_base);
    set_tzcc_base(g_da_ctx.tzcc_base);

    printf("SEJ base: 0x%08" PRIx32 "\n", g_da_ctx.sej_base);
    printf("TZCC base: 0x%08" PRIx32 "\n", g_da_ctx.tzcc_base);
    printf("Read packet size: 0x%08" PRIx32 "\n", g_da_ctx.read_packet_size);
    printf("Write packet size: 0x%08" PRIx32 "\n", g_da_ctx.write_packet_size);

    if (g_da_ctx.storage == STORAGE_EMMC) {
        printf("Storage type: eMMC\n");
        rpmb_mmc_setup(mmc_get_card);
    } else {
        printf("Unsupported storage type in DA context: %u\n", g_da_ctx.storage);
        g_da_ctx.storage = STORAGE_UNKNOWN;
    }

    printf("DA context setup complete!\n");

    return 0;
}

int cmd_readmem(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    address_range_t range = {0};
    uint32_t range_size = sizeof(range);


    status = channel->read((uint8_t*)&range, &range_size);
    channel->write((uint8_t*)&status, 4);
    if (status != 0) {
        printf("Failed to read memory range!\n");
        return status;
    }

    printf("Reading memory: address=0x%08" PRIx32 " length=0x%" PRIx32 "\n", range.start, range.length);

    uint8_t* src = (uint8_t*)(uintptr_t)range.start;
    status = upload_data(channel, src, range.length, "Memory Read");

    return status;
}


int cmd_writemem(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    address_range_t range = {0};
    uint32_t range_size = sizeof(range);



    status = channel->read((uint8_t*)&range, &range_size);
    channel->write((uint8_t*)&status, 4);
    if (status != 0) {
        printf("Failed to read memory range!\n");
        return status;
    }

    printf("Writing memory: address=0x%08" PRIx32 " length=0x%" PRIx32 "\n", range.start, range.length);

    uint8_t* dest = (uint8_t*)(uintptr_t)range.start;
    status = download_data(channel, &dest, range.length, "Memory Write");

    return status;
}

int cmd_readregister(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    uint32_t reg_addr = 0;
    uint32_t size = 4;

    printf("Reading register: address=0x%08" PRIx32 "\n", reg_addr);

    status = channel->read((uint8_t*)&reg_addr, &size);
    return channel->write((uint8_t*)&status, 4);
}

int cmd_writeregister(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    uint32_t reg_addr = 0;
    uint32_t reg_value = 0;
    uint32_t size = 4;

    status = channel->read((uint8_t*)&reg_addr, &size);
    status = channel->read((uint8_t*)&reg_value, &size);

    printf("Writing register: address=0x%08" PRIx32 " value=0x%08" PRIx32 "\n", reg_addr, reg_value);

    return channel->write((uint8_t*)&status, 4);
}

int cmd_key_derive(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    uint32_t length = 4;
    uint8_t key[32] __attribute__((aligned(16))) = {0};
    uint32_t key_length = 0x20;
    int key_type = 0;

    status = channel->read((uint8_t*)&key_type, &length);
    channel->write((uint8_t*)&status, length);

    if (key_type == FDE_KEY) {
        key_length = 0x10;
    }

    printf("Deriving key of type %d\n", key_type);

    status = (int)key_derive(key_type, key, key_length);

    return channel->write(key, key_length);
}

int cmd_sej_aes(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    /*
     * Format:
     * 0100010020000000
     * Byte 0: Encrypt (1) / Decrypt (0)
     * Byte 1: Legacy SEJ (1) / Non-Legacy SEJ (0)
     * Byte 2: Anti-clone enable (1) / disable (0)
     * Byte 3: Reserved
     * Bytes 4-7: Data length (little-endian)
     *
     */
    #define AES_MAX_LEN 8192
    int status = 0;
    uint8_t params[8] = {0};
    bool encrypt = false;
    bool legacy = false;
    bool anti_clone = true;

    uint8_t* data_buf = NULL;

    uint32_t param_len = sizeof(params);
    channel->read(params, &param_len);
    channel->write((uint8_t*)&status, 4);

    encrypt = params[0];
    legacy  = params[1];
    anti_clone = params[2];
    uint32_t length = *(uint32_t*)&params[4];

    if (length > AES_MAX_LEN) {
        status = STATUS_SEJ_EXCEED_MAX_LEN;
        return status;
    }

    printf("SEJ AES: encrypt=%d legacy=%d anti_clone=%d length=0x%" PRIx32 "\n", encrypt, legacy, anti_clone, length);

    status = download_data(channel, &data_buf, length, "SEJ AES Data");
    if (status != 0) {
        if (status == STATUS_MALLOC_FAILED)
            return status;

        goto out;
    }

    if (encrypt)
        sp_sej_enc(data_buf, data_buf, length, anti_clone, legacy);
    else
        sp_sej_dec(data_buf, data_buf, length, anti_clone, legacy);

    upload_data(channel, data_buf, length, "SEJ AES Result");

out:
    free(data_buf);
    return status;
}

int cmd_rpmb_init(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    uint32_t size = 0x20;
    uint32_t status_len = 4;
    uint32_t rpmb_part = 0;
    uint8_t rpmbkey[0x20];

    channel->read((uint8_t*)&rpmb_part, &status_len);
    channel->read((uint8_t*)rpmbkey, &size);

    if (g_da_ctx.storage == STORAGE_UNKNOWN) {
        printf("Storage type unknown, cannot initialize RPMB!\n");
        status = STATUS_RPMB_STORAGE_NOT_SUPPORTED;
        goto out;
    }

    if (rpmb_is_initialized(rpmb_part)) {
        printf("RPMB partition %u already initialized! Skipping\n", rpmb_part);
        goto out;
    }

    printf("Setting RPMB key for partition %u\n", rpmb_part);
    rpmb_set_key(rpmb_part, rpmbkey);

    printf("Initializing RPMB partition %u\n", rpmb_part);
    if (rpmb_init(rpmb_part) < 0)
        status = STATUS_RPMB_KEY_INVALID;

out:
    channel->write((uint8_t*)&status, status_len);
    return 0;
}

int cmd_rpmb_read(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    uint32_t status_sz = 4;
    uint32_t rpmb_part = 0;
    storage_range_t sector = {0};
    uint32_t sector_sz = sizeof(sector);

    status = channel->read((uint8_t*)&rpmb_part, &status_sz);
    status = channel->read((uint8_t*)&sector, &sector_sz);

    if (g_da_ctx.storage == STORAGE_UNKNOWN) {
        printf("Storage type unknown, cannot read RPMB!\n");
        status = STATUS_RPMB_STORAGE_NOT_SUPPORTED;
        channel->write((uint8_t*)&status, status_sz);
        return status;
    }

    if (rpmb_is_initialized(rpmb_part) == false) {
        printf("RPMB partition %u not initialized!\n", rpmb_part);
        status = STATUS_RPMB_NOT_INIT;
        channel->write((uint8_t*)&status, status_sz);
        return status;
    }

    channel->write((uint8_t*)&status, status_sz);

    uint32_t data_len = sector.sector_count * RPMB_DATA_SZ;

    printf("RPMB: Reading 0x%" PRIx32 " bytes from partition %u, starting at sector %u\n", data_len, rpmb_part, sector.start_sector);

    struct rpmb_stream_ctx rctx = { .rpmb_part = rpmb_part, .start_sector = sector.start_sector };
    status = upload_data_stream(channel, data_len, 0, rpmb_read_stream_cb, &rctx, "RPMB Read");

    if (status == 0) {
        printf("Finished reading RPMB\n");
    } else if (status != STATUS_MALLOC_FAILED && status != STATUS_UPLOAD_ACK_NOT_OK) {
        printf("RPMB read failed with error %d\n", status);
        status = STATUS_RPMB_READ_FAILED;
    }

    return status;
}

int cmd_rpmb_write(com_channel_struct *channel) {
    printf("\n\n*** Enter %s cmd ***\n\n", __func__);

    int status = 0;
    uint32_t status_sz = 4;
    uint32_t rpmb_part = 0;
    storage_range_t sector = {0};
    uint32_t sector_sz = sizeof(sector);

    status = channel->read((uint8_t*)&rpmb_part, &status_sz);
    status = channel->read((uint8_t*)&sector, &sector_sz);

    if (g_da_ctx.storage == STORAGE_UNKNOWN) {
        printf("Storage type unknown, cannot write RPMB!\n");
        status = STATUS_RPMB_STORAGE_NOT_SUPPORTED;
        channel->write((uint8_t*)&status, status_sz);
        return status;
    }

    if (rpmb_is_initialized(rpmb_part) == false) {
        printf("RPMB partition %u not initialized!\n", rpmb_part);
        status = STATUS_RPMB_NOT_INIT;
        channel->write((uint8_t*)&status, status_sz);
        return status;
    }

    channel->write((uint8_t*)&status, status_sz);

    uint32_t data_len = sector.sector_count * RPMB_DATA_SZ;

    printf("RPMB: Writing 0x%" PRIx32 " bytes to partition %u, starting at sector %u\n", data_len, rpmb_part, sector.start_sector);

    struct rpmb_stream_ctx rctx = { .rpmb_part = rpmb_part, .start_sector = sector.start_sector };
    status = download_data_stream(channel, data_len, 32 * 1024, rpmb_write_stream_cb, &rctx, "RPMB Write");

    if (status == 0) {
        printf("Finished writing RPMB\n");
    } else if (status != STATUS_MALLOC_FAILED && status != STATUS_DOWNLOAD_ACK_NOT_OK) {
        printf("RPMB write failed with error %d\n", status);
        status = STATUS_RPMB_WRITE_FAILED;
    }

    return status;
}

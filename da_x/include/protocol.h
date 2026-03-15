/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_X_PROTOCOL_H
#define DA_X_PROTOCOL_H

#include <hal.h>
#include <stdint.h>
#include <stream.h>

#define FACILITY_EXTENSIONS 0xE

#define SEV_SUCCESS 	(0 << 30)
#define SEV_INFO 		(1 << 30)
#define SEV_WARNING 	(2 << 30)
#define SEV_ERROR 		(3 << 30)

#define MAKE_CODE(sev, fac, code) ((sev) | (fac << 16) | (code))

#define STATUS_OK (0x00000000)

#define STATUS_DOWNLOAD_ACK_NOT_OK          (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x01))
#define STATUS_UPLOAD_ACK_NOT_OK            (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x02))
#define STATUS_SEJ_EXCEED_MAX_LEN           (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x03))

#define STATUS_MALLOC_FAILED                (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x04))
#define STATUS_RPMB_NOT_INIT                (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x05))
#define STATUS_RPMB_READ_FAILED             (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x06))
#define STATUS_RPMB_WRITE_FAILED            (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x07))
#define STATUS_RPMB_KEY_INVALID             (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x08))
#define STATUS_RPMB_STORAGE_NOT_SUPPORTED   (MAKE_CODE(SEV_ERROR, FACILITY_EXTENSIONS, 0x09))

extern com_channel_struct* g_com_channel;

extern void *(*malloc)(size_t size);
extern void (*free)(void *ptr);
extern void *(*mmc_get_card)(int card_id);

void da_log_register(com_channel_struct *channel);
int download_data(com_channel_struct* channel, uint8_t** dst, uint64_t size, const char* desc);
int upload_data(com_channel_struct* channel, const uint8_t* src, uint64_t size, const char* desc);

int download_data_stream(com_channel_struct* channel, uint64_t size, uint32_t chunk_size, data_stream_cb cb, void *ctx, const char* desc);
int upload_data_stream(com_channel_struct* channel, uint64_t size, uint32_t chunk_size, data_stream_cb cb, void *ctx, const char* desc);

#endif //DA_X_PROTOCOL_H

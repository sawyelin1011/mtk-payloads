/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_X_HAL_H
#define DA_X_HAL_H

#include <stdint.h>

typedef struct {
    int (*read)(uint8_t* buffer, uint32_t* length);
    int (*write)(uint8_t* buffer, uint32_t length);
    int (*log_to_pc)(const uint8_t* buffer, uint32_t length);
    int (*log_to_uart)(const uint8_t* buffer, uint32_t length);
} com_channel_struct;

typedef int (*HHANDLE)(com_channel_struct*);

#endif //DA_X_COM_H

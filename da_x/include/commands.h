/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_X_COMMANDS_H
#define DA_X_COMMANDS_H

#include <da.h>
#include <hal.h>

int cmd_ack(com_channel_struct *channel);
int cmd_setup_da_ctx(com_channel_struct *channel);
int cmd_readmem(com_channel_struct *channel);
int cmd_writemem(com_channel_struct *channel);
int cmd_readregister(com_channel_struct *channel);
int cmd_writeregister(com_channel_struct *channel);
int cmd_key_derive(com_channel_struct *channel);
int cmd_sej_aes(com_channel_struct *channel);
int cmd_rpmb_init(com_channel_struct *channel);
int cmd_rpmb_read(com_channel_struct *channel);
int cmd_rpmb_write(com_channel_struct *channel);

#endif //DA_X_COMMANDS_H

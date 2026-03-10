/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef DA_XML_COMMANDS_H
#define DA_XML_COMMANDS_H

#include <protocol_functions.h>

#define CMD_ACK "CMD:EXT-ACK"
#define CMD_DA_CTX "CMD:EXT-DA-CTX"
#define CMD_SEJ "CMD:EXT-SEJ"
#define CMD_SEJ_BASE "CMD:EXT-SET-SEJ-BASE"
#define CMD_READMEM "CMD:EXT-READ-MEM"
#define CMD_WRITEMEM "CMD:EXT-WRITE-MEM"


int cmd_ack(struct com_channel_struct *channel, const char *xml);
int cmd_readmem(struct com_channel_struct *channel, const char *xml);
int cmd_writemem(struct com_channel_struct *channel, const char *xml);
int cmd_sej_aes(struct com_channel_struct *channel, const char *xml);
int cmd_set_sej_base(struct com_channel_struct*, const char* xml);

#endif //DA_XML_COMMANDS_H

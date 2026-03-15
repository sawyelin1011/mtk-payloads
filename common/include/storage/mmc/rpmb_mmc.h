/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef RPMB_MMC_H
#define RPMB_MMC_H

#include <security/rpmb.h>
#include <storage/mmc/mmc.h>

int rpmb_mmc_setup(void *(*mmc_get_card)(int id));
int rpmb_mmc_setup_dev(struct mmc_dev *dev);

struct mmc_dev *rpmb_mmc_get_dev(void);

#endif /* RPMB_MMC_H */

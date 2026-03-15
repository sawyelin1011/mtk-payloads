/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#ifndef MMC_H
#define MMC_H

#include <stdint.h>
#include <stdbool.h>

#define MSDC_CFG            0x00
#define MSDC_INT            0x0C
#define MSDC_FIFOCS         0x14
#define MSDC_TXDATA         0x18
#define MSDC_RXDATA         0x1C
#define SDC_CFG             0x30
#define SDC_CMD             0x34
#define SDC_ARG             0x38
#define SDC_STS             0x3C
#define SDC_RESP0           0x40
#define SDC_RESP1           0x44
#define SDC_RESP2           0x48
#define SDC_RESP3           0x4C
#define SDC_BLK_NUM         0x50
#define EMMC_CFG0           0x70

#define MSDC_INT_CMDRDY         (1u << 8)
#define MSDC_INT_CMDTMO         (1u << 9)
#define MSDC_INT_RSPCRCERR      (1u << 10)
#define MSDC_INT_XFER_COMPL     (1u << 12)
#define MSDC_INT_DATTMO         (1u << 14)
#define MSDC_INT_DATCRCERR      (1u << 15)

#define MSDC_FIFOCS_RXCNT       0x000000FFu
#define MSDC_FIFOCS_TXCNT       0x00FF0000u
#define MSDC_FIFOCS_CLR         (1u << 31)

#define SDC_STS_SDCBUSY         (1u << 0)
#define SDC_STS_CMDBUSY         (1u << 1)

#define MSDC_CFG_MODE           (1u << 0)
#define MSDC_CFG_PIO            (1u << 3)
#define MSDC_CFG_CKSTB          (1u << 7)

#define EMMC_CFG0_BOOTSUPP      (1u << 0)

#define MMC_GO_IDLE_STATE       0
#define MMC_SEND_OP_COND        1
#define MMC_ALL_SEND_CID        2
#define MMC_SET_RELATIVE_ADDR   3
#define MMC_SWITCH              6
#define MMC_SELECT_CARD         7
#define MMC_SEND_EXT_CSD        8
#define MMC_SEND_STATUS         13
#define MMC_SET_BLOCKLEN        16
#define MMC_READ_SINGLE_BLOCK   17
#define MMC_READ_MULTIPLE_BLOCK 18
#define MMC_SET_BLOCK_COUNT     23
#define MMC_WRITE_BLOCK         24
#define MMC_WRITE_MULTIPLE_BLOCK 25

#define MMC_RSP_NONE    0
#define MMC_RSP_R1      1
#define MMC_RSP_R2      2
#define MMC_RSP_R3      3
#define MMC_RSP_R1B     7

#define EXT_CSD_PART_CONFIG         179
#define EXT_CSD_BUS_WIDTH           183
#define EXT_CSD_HS_TIMING           185
#define EXT_CSD_REV                 192
#define EXT_CSD_SEC_CNT             212
#define EXT_CSD_RPMB_SIZE_MULT     168

#define EXT_CSD_PART_ACCESS_MASK    0x07
#define EXT_CSD_PART_BOOT_MASK      0xF8

#define MMC_PART_USER   0
#define MMC_PART_BOOT0  1
#define MMC_PART_BOOT1  2
#define MMC_PART_RPMB   3

#define MMC_SWITCH_MODE_WRITE_BYTE  3

#define MMC_SWITCH_ARG(index, value) \
    ((uint32_t)(MMC_SWITCH_MODE_WRITE_BYTE) << 24 | \
     (uint32_t)(index) << 16 | \
     (uint32_t)(value) << 8)

#define CMD23_RELIABLE_WRITE    ((uint32_t)0x80000000UL)

#define RPMB_PROGRAM_KEY        1
#define RPMB_GET_WRITE_COUNTER  2
#define RPMB_WRITE_DATA         3
#define RPMB_READ_DATA          4
#define RPMB_RESULT_READ        5

#define RPMB_REQ                1
#define RPMB_RESP               2

#define MMC_CARD_BUSY           (1u << 31)
#define MMC_FIFO_SZ             128
#define MMC_BLOCK_SZ            512

#define R1_SWITCH_ERROR         (1u << 7)
#define R1_READY_FOR_DATA       (1u << 8)
#define R1_CURRENT_STATE_MASK   0x00001E00u
#define R1_CURRENT_STATE_SHIFT  9
#define R1_STATE_TRAN           4
#define R1_STATE_PRG            7

struct mmc_host {
    void    *card;
    uint64_t max_hw_segs;
    uint64_t max_phys_segs;
    uint64_t max_seg_size;
    uint32_t max_blk_size;
    uint32_t max_blk_count;
    uint32_t base;
    // TODO: Add the rest ;D
};

struct mmc_card {
    struct mmc_host *host;
    uint32_t nblks;
    uint32_t blklen;
    uint32_t ocr;
    uint32_t maxhz;
    uint32_t uhs_mode;
    uint32_t rca;
    uint32_t type;
    uint16_t state;
    uint16_t ready;
    uint32_t raw_cid[4];
    uint32_t raw_csd[4];
    uint32_t raw_scr[2];
    uint8_t  raw_ext_csd[512];
};

struct mmc_dev {
    uintptr_t base;
    uint32_t  rca;
    uint8_t  *ext_csd;
    uint8_t   ext_csd_local[512];
    bool      attached; // If we used the original DA card struct, or we initialized from scratch
};

/*
 * Attach to an already existing card struct (i.e. DA or lk).
 * The hardware is assumed to be fully initialized.
 */
int mmc_dev_from_card(struct mmc_dev *dev, void *in_card);

/*
 * Like the above, but with explicit parameters.
 */
void mmc_dev_setup(struct mmc_dev *dev, uintptr_t base, uint32_t rca, const uint8_t *ext_csd);

/*
 * Full eMMC initialization from scratch at the given MSDC base.
 * Performs CMD0 -> CMD1 -> CMD2 -> CMD3 -> CMD7 -> CMD8
 */
int mmc_dev_init(struct mmc_dev *dev, uintptr_t base);


void mmc_hw_reset(struct mmc_dev *dev);
int mmc_cmd(struct mmc_dev *dev, uint32_t opcode, uint32_t arg,
            uint32_t resp, int dtype, int rw, uint32_t *resp_out);

int mmc_pio_read(struct mmc_dev *dev, uint8_t *buf, uint32_t sz);
int mmc_pio_write(struct mmc_dev *dev, const uint8_t *buf, uint32_t sz);

int mmc_read_ext_csd(struct mmc_dev *dev, uint8_t *ext_csd);
int mmc_send_status(struct mmc_dev *dev, uint32_t *status);
int mmc_wait_ready(struct mmc_dev *dev);
int mmc_switch(struct mmc_dev *dev, uint8_t index, uint8_t value);
int mmc_switch_part(struct mmc_dev *dev, uint8_t part);

int mmc_read_block(struct mmc_dev *dev, uint32_t blk, void *buf);
int mmc_write_block(struct mmc_dev *dev, uint32_t blk, const void *buf);

int mmc_rpmb_send(struct mmc_dev *dev, uint8_t *frame, uint16_t blks, int type, int is_req);

#endif /* MMC_H */

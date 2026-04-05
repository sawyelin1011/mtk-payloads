/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#include <stdint.h>
#include <storage/mmc/mmc.h>
#include <mmio.h>
#include <libc.h>
#include <debug.h>
#include <stddef.h>

#define REG(dev, off)   ((uintptr_t)(dev)->base + (off))

#define bswap16 __builtin_bswap16
#define bswap32 __builtin_bswap32

// I bricked my EXT_CSD too much
#define DEFAULT_PART_CFG 0x48

void mmc_hw_reset(struct mmc_dev *dev) {
    uint32_t val;

    /* Flush FIFO */
    setbits_32(REG(dev, MSDC_FIFOCS), MSDC_FIFOCS_CLR);
    readl_poll_timeout(REG(dev, MSDC_FIFOCS), val,
                       !(val & MSDC_FIFOCS_CLR), 100000);

    /* Clear all pending interrupts */
    writel_relaxed(readl_relaxed(REG(dev, MSDC_INT)), REG(dev, MSDC_INT));
}

int mmc_cmd(struct mmc_dev *dev, uint32_t opcode, uint32_t arg, uint32_t resp, int dtype, int rw, uint32_t *resp_out) {
    uint32_t val, rawcmd, mask, ints;

    /* Wait for controller to be idle */
    if (readl_poll_timeout(REG(dev, SDC_STS), val,
                           !(val & SDC_STS_SDCBUSY), 1000000)) {
        printf("[MMC] cmd%u: SDC busy timeout\n", opcode);
        mmc_hw_reset(dev);
        return -1;
    }

    rawcmd = opcode | ((resp & 0x7) << 7);

    if (dtype) {
        rawcmd |= ((uint32_t)(dtype & 3) << 11);
        rawcmd |= (0x200u << 16);
        if (rw)
            rawcmd |= (1u << 13);
    }

    writel_relaxed(readl_relaxed(REG(dev, MSDC_INT)), REG(dev, MSDC_INT));
    writel_relaxed(arg, REG(dev, SDC_ARG));
    wmb();
    writel_relaxed(rawcmd, REG(dev, SDC_CMD));

    mask = MSDC_INT_CMDRDY | MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR;
    if (readl_poll_timeout(REG(dev, MSDC_INT), ints,
                           (ints & mask), 1000000)) {
        printf("[MMC] cmd%u: completion timeout\n", opcode);
        mmc_hw_reset(dev);
        return -1;
    }
    writel_relaxed(ints & mask, REG(dev, MSDC_INT));

    if (ints & (MSDC_INT_CMDTMO | MSDC_INT_RSPCRCERR)) {
        printf("[MMC] cmd%u: error (ints=0x%08x)\n", opcode, ints);
        mmc_hw_reset(dev);
        return -1;
    }

    if (resp_out)
        *resp_out = readl_relaxed(REG(dev, SDC_RESP0));

    return 0;
}

int mmc_pio_read(struct mmc_dev *dev, uint8_t *buf, uint32_t sz) {
    uint32_t *p = (uint32_t *)buf;
    uint32_t left = sz;
    uint32_t timeout = 1000000;

    while (timeout--) {
        uint32_t ints = readl_relaxed(REG(dev, MSDC_INT));

        if (ints & (MSDC_INT_DATTMO | MSDC_INT_DATCRCERR)) {
            printf("[MMC] %s: data error (ints=0x%08x)\n", __func__, ints);
            mmc_hw_reset(dev);
            return -1;
        }

        uint32_t cnt = (readl_relaxed(REG(dev, MSDC_FIFOCS))
                        & MSDC_FIFOCS_RXCNT);

        uint32_t c = (cnt < left ? cnt : left) >> 2;
        if (c > 0) {
            volatile uint32_t *rx_reg = (volatile uint32_t *)REG(dev, MSDC_RXDATA);
            uint32_t n = c << 2;
            while (c >= 16) {
                p[0] = *rx_reg;
                p[1] = *rx_reg;
                p[2] = *rx_reg;
                p[3] = *rx_reg;
                p[4] = *rx_reg;
                p[5] = *rx_reg;
                p[6] = *rx_reg;
                p[7] = *rx_reg;
                p[8] = *rx_reg;
                p[9] = *rx_reg;
                p[10] = *rx_reg;
                p[11] = *rx_reg;
                p[12] = *rx_reg;
                p[13] = *rx_reg;
                p[14] = *rx_reg;
                p[15] = *rx_reg;
                p += 16;
                c -= 16;
            }
            while (c >= 8) {
                p[0] = *rx_reg;
                p[1] = *rx_reg;
                p[2] = *rx_reg;
                p[3] = *rx_reg;
                p[4] = *rx_reg;
                p[5] = *rx_reg;
                p[6] = *rx_reg;
                p[7] = *rx_reg;
                p += 8;
                c -= 8;
            }
            while (c > 0) {
                *p++ = *rx_reg;
                c--;
            }
            left -= n;
            timeout = 1000000;
        }

        if (!left)
            break;

        if ((ints & MSDC_INT_XFER_COMPL) && (readl_relaxed(REG(dev, MSDC_FIFOCS)) & MSDC_FIFOCS_RXCNT) < 4)
            break;
    }

    if (timeout == 0) {
        printf("[MMC] %s: timeout\n", __func__);
        mmc_hw_reset(dev);
        return -1;
    }

    uint32_t val;
    if (readl_poll_timeout(REG(dev, MSDC_INT), val,
                           (val & MSDC_INT_XFER_COMPL), 1000000)) {
        printf("[MMC] %s: XFER_COMPL timeout\n", __func__);
        mmc_hw_reset(dev);
        return -1;
    }
    writel_relaxed(MSDC_INT_XFER_COMPL, REG(dev, MSDC_INT));

    if (left > 0) {
        printf("[MMC] %s: early end (left=%u)\n", __func__, left);
        return -1;
    }

    return 0;
}

int mmc_pio_write(struct mmc_dev *dev, const uint8_t *buf, uint32_t sz) {
    const uint32_t *p = (const uint32_t *)buf;
    uint32_t left = sz;
    uint32_t timeout = 1000000;

    while (left > 0 && timeout--) {
        uint32_t ints = readl_relaxed(REG(dev, MSDC_INT));

        if (ints & (MSDC_INT_DATTMO | MSDC_INT_DATCRCERR)) {
            printf("[MMC] %s: data error (ints=0x%08x)\n", __func__, ints);
            mmc_hw_reset(dev);
            return -1;
        }

        uint32_t tx_cnt = (readl_relaxed(REG(dev, MSDC_FIFOCS))
                           & MSDC_FIFOCS_TXCNT) >> 16;

        if (tx_cnt == 0) {
            uint32_t n = (left < MMC_FIFO_SZ ? left : MMC_FIFO_SZ) & ~3u;
            uint32_t c = n >> 2;
            volatile uint32_t *tx_reg = (volatile uint32_t *)REG(dev, MSDC_TXDATA);
            while (c >= 16) {
                *tx_reg = p[0];
                *tx_reg = p[1];
                *tx_reg = p[2];
                *tx_reg = p[3];
                *tx_reg = p[4];
                *tx_reg = p[5];
                *tx_reg = p[6];
                *tx_reg = p[7];
                *tx_reg = p[8];
                *tx_reg = p[9];
                *tx_reg = p[10];
                *tx_reg = p[11];
                *tx_reg = p[12];
                *tx_reg = p[13];
                *tx_reg = p[14];
                *tx_reg = p[15];
                p += 16;
                c -= 16;
            }
            while (c >= 8) {
                *tx_reg = p[0];
                *tx_reg = p[1];
                *tx_reg = p[2];
                *tx_reg = p[3];
                *tx_reg = p[4];
                *tx_reg = p[5];
                *tx_reg = p[6];
                *tx_reg = p[7];
                p += 8;
                c -= 8;
            }
            while (c > 0) {
                *tx_reg = *p++;
                c--;
            }
            left -= n;
            timeout = 1000000;
        }
    }

    if (timeout == 0) {
        printf("[MMC] %s: timeout\n", __func__);
        mmc_hw_reset(dev);
        return -1;
    }

    uint32_t val;
    if (readl_poll_timeout(REG(dev, MSDC_INT), val,
                           (val & MSDC_INT_XFER_COMPL), 1000000)) {
        printf("[MMC] %s: XFER_COMPL timeout\n", __func__);
        mmc_hw_reset(dev);
        return -1;
    }
    writel_relaxed(MSDC_INT_XFER_COMPL, REG(dev, MSDC_INT));
    return 0;
}

int mmc_send_status(struct mmc_dev *dev, uint32_t *status) {
    return mmc_cmd(dev, MMC_SEND_STATUS, (uint32_t)dev->rca << 16, MMC_RSP_R1, 0, 0, status);
}

int mmc_wait_ready(struct mmc_dev *dev) {
    for (int i = 0; i < 100000; i++) {
        uint32_t status;
        if (mmc_send_status(dev, &status))
            return -1;
        uint32_t state = (status & R1_CURRENT_STATE_MASK) >> R1_CURRENT_STATE_SHIFT;
        if (state == R1_STATE_TRAN)
            return 0;
    }
    printf("[MMC] %s: timeout\n", __func__);
    return -1;
}

int mmc_switch(struct mmc_dev *dev, uint8_t index, uint8_t value) {
    uint32_t arg = MMC_SWITCH_ARG(index, value);

    if (mmc_cmd(dev, MMC_SWITCH, arg, MMC_RSP_R1B, 0, 0, NULL))
        return -1;

    return mmc_wait_ready(dev);
}

int mmc_switch_part(struct mmc_dev *dev, uint8_t part) {

    uint8_t cur_cfg = dev->ext_csd ? dev->ext_csd[EXT_CSD_PART_CONFIG] : DEFAULT_PART_CFG;
    uint8_t new_cfg = (cur_cfg & EXT_CSD_PART_BOOT_MASK) | (part & EXT_CSD_PART_ACCESS_MASK);

    if (cur_cfg == new_cfg)
        return 0;

    int ret = mmc_switch(dev, EXT_CSD_PART_CONFIG, new_cfg);
    if (ret == 0 && dev->ext_csd)
        dev->ext_csd[EXT_CSD_PART_CONFIG] = new_cfg;

    return ret;
}

int mmc_read_block(struct mmc_dev *dev, uint32_t blk, void *buf) {
    writel_relaxed(1, REG(dev, SDC_BLK_NUM));

    if (mmc_cmd(dev, MMC_READ_SINGLE_BLOCK, blk,
                MMC_RSP_R1, 1, 0, NULL))
        return -1;

    return mmc_pio_read(dev, (uint8_t *)buf, MMC_BLOCK_SZ);
}

int mmc_write_block(struct mmc_dev *dev, uint32_t blk, const void *buf) {
    writel_relaxed(1, REG(dev, SDC_BLK_NUM));

    if (mmc_cmd(dev, MMC_WRITE_BLOCK, blk, MMC_RSP_R1, 1, 1, NULL))
        return -1;

    return mmc_pio_write(dev, (const uint8_t *)buf, MMC_BLOCK_SZ);
}

int mmc_read_blocks(struct mmc_dev *dev, uint32_t blk, void *buf, uint32_t count) {
    if (count == 0)
        return 0;

    if (count == 1)
        return mmc_read_block(dev, blk, buf);

    writel_relaxed(count, REG(dev, SDC_BLK_NUM));
    if (mmc_cmd(dev, MMC_SET_BLOCK_COUNT, count, MMC_RSP_R1, 0, 0, NULL))
        return -1;

    writel_relaxed(count, REG(dev, SDC_BLK_NUM));
    if (mmc_cmd(dev, MMC_READ_MULTIPLE_BLOCK, blk, MMC_RSP_R1, 2, 0, NULL))
        return -1;

    return mmc_pio_read(dev, (uint8_t *)buf, MMC_BLOCK_SZ * count);
}

int mmc_write_blocks(struct mmc_dev *dev, uint32_t blk, const void *buf, uint32_t count) {
    if (count == 0)
        return 0;

    if (count == 1)
        return mmc_write_block(dev, blk, buf);

    writel_relaxed(count, REG(dev, SDC_BLK_NUM));
    if (mmc_cmd(dev, MMC_SET_BLOCK_COUNT, count, MMC_RSP_R1, 0, 0, NULL))
        return -1;

    writel_relaxed(count, REG(dev, SDC_BLK_NUM));
    if (mmc_cmd(dev, MMC_WRITE_MULTIPLE_BLOCK, blk, MMC_RSP_R1, 2, 1, NULL))
        return -1;

    int ret = mmc_pio_write(dev, (const uint8_t *)buf, MMC_BLOCK_SZ * count);
    if (ret)
        return ret;

    return mmc_wait_ready(dev);
}

int mmc_rpmb_send(struct mmc_dev *dev, uint8_t *frame, uint16_t blks, int type, int is_req) {
    uint32_t cmd23_arg = (uint32_t)blks;
    uint32_t xfer_sz   = MMC_BLOCK_SZ * (uint32_t)blks;

    if (is_req && (type == RPMB_WRITE_DATA || type == RPMB_PROGRAM_KEY))
        cmd23_arg |= CMD23_RELIABLE_WRITE;

    writel_relaxed((uint32_t)blks, REG(dev, SDC_BLK_NUM));
    if (mmc_cmd(dev, MMC_SET_BLOCK_COUNT, cmd23_arg, MMC_RSP_R1, 0, 0, NULL))
        return -1;

    writel_relaxed((uint32_t)blks, REG(dev, SDC_BLK_NUM));

    if (is_req) {
        if (mmc_cmd(dev, MMC_WRITE_MULTIPLE_BLOCK, 0, MMC_RSP_R1, 2, 1, NULL))
            return -1;

        return mmc_pio_write(dev, frame, xfer_sz);
    } else {
        if (mmc_cmd(dev, MMC_READ_MULTIPLE_BLOCK, 0, MMC_RSP_R1, 2, 0, NULL))
            return -1;

        return mmc_pio_read(dev, frame, xfer_sz);
    }
}

void mmc_dev_setup(struct mmc_dev *dev, uintptr_t base, uint32_t rca, const uint8_t *ext_csd) {
    memset(dev, 0, sizeof(*dev));

    dev->base     = base;
    dev->rca      = rca;
    dev->attached = true;

    if (ext_csd) {
        dev->ext_csd = (uint8_t *)ext_csd;
    } else {
        dev->ext_csd = dev->ext_csd_local;

        if (mmc_read_ext_csd(dev, dev->ext_csd_local) == 0) {
            printf("[MMC] EXT_CSD read OK, part_cfg=0x%02x\n",
                   dev->ext_csd_local[EXT_CSD_PART_CONFIG]);
        } else {
            printf("[MMC] EXT_CSD read failed\n");
            dev->ext_csd = NULL;
        }
    }
}

uintptr_t find_mmc_base(void *host) {
    uint32_t *ptr = (uint32_t *)host;
    if (!ptr) return 0;

    for (int i = 0; i < 64; i++) {
        uint32_t val = ptr[i];
        if ((val & 0xFF000000) == 0x11000000) {
            return val;
        }
    }
    return 0;
}

int mmc_dev_from_card(struct mmc_dev *dev, void *in_card) {
    if (!in_card) {
        printf("[MMC] %s: NULL card pointer\n", __func__);
        return -1;
    }

    struct mmc_card *card = (struct mmc_card *)in_card;
    uintptr_t base = 0;
    uint32_t rca   = 1;

    if (card->rca)
        rca = card->rca;

    base = find_mmc_base(card->host);

    if (!base) {
        printf("[MMC] %s: could not determine MSDC base\n", __func__);
        return -1;
    }

    memset(dev, 0, sizeof(*dev));
    dev->base     = base;
    dev->rca      = rca;
    dev->attached = true;


    dev->ext_csd = (uint8_t *)card->raw_ext_csd;

    return 0;
}

int mmc_read_ext_csd(struct mmc_dev *dev, uint8_t *ext_csd) {
    writel_relaxed(1, REG(dev, SDC_BLK_NUM));

    if (mmc_cmd(dev, MMC_SEND_EXT_CSD, 0, MMC_RSP_R1, 1, 0, NULL))
        return -1;

    return mmc_pio_read(dev, ext_csd, MMC_BLOCK_SZ);
}

int mmc_dev_init(struct mmc_dev *dev, uintptr_t base) {
    int ret;
    uint32_t val;

    memset(dev, 0, sizeof(*dev));
    dev->base     = base;
    dev->rca      = 1;
    dev->attached = false;
    dev->ext_csd  = dev->ext_csd_local;

    printf("[MMC] %s: base=0x%08x\n", __func__, (unsigned int)base);

    /* Set SD/MMC mode, PIO mode */
    val = readl_relaxed(REG(dev, MSDC_CFG));
    val |= MSDC_CFG_MODE | MSDC_CFG_PIO;
    writel_relaxed(val, REG(dev, MSDC_CFG));

    /* Disable boot mode */
    clrbits_32(REG(dev, EMMC_CFG0), EMMC_CFG0_BOOTSUPP);

    /* Reset + clear FIFO */
    mmc_hw_reset(dev);

    /* Wait for clock stable */
    readl_poll_timeout(REG(dev, MSDC_CFG), val, (val & MSDC_CFG_CKSTB), 100000);

    /* Let it think!! */
    for (volatile int d = 0; d < 50000; d++);

    ret = mmc_cmd(dev, MMC_GO_IDLE_STATE, 0, MMC_RSP_NONE, 0, 0, NULL);
    if (ret) {
        printf("[MMC] %s: GO_IDLE failed (%d)\n", __func__, ret);
        return ret;
    }

    printf("[MMC] %s: GO_IDLE ok\n", __func__);

    uint32_t ocr = 0, rocr = 0;
    ret = mmc_cmd(dev, MMC_SEND_OP_COND, 0, MMC_RSP_R3, 0, 0, &ocr);
    if (ret) {
        printf("[MMC] %s: SEND_OP_COND probe failed (%d)\n", __func__, ret);
        return ret;
    }

    printf("[MMC] %s: OCR=0x%08x\n", __func__, ocr);

    uint32_t host_ocr = (ocr & 0x00FF8000) | (1u << 30);

    for (int i = 0; i < 200; i++) {
        ret = mmc_cmd(dev, MMC_SEND_OP_COND, host_ocr, MMC_RSP_R3, 0, 0, &rocr);
        if (ret)
            return ret;
        if (rocr & MMC_CARD_BUSY)
            break;
        for (volatile int d = 0; d < 10000; d++);
    }

    if (!(rocr & MMC_CARD_BUSY)) {
        printf("[MMC] %s: card did not become ready\n", __func__);
        return -1;
    }
    printf("[MMC] %s: card ready, OCR=0x%08x\n", __func__, rocr);

    ret = mmc_cmd(dev, MMC_ALL_SEND_CID, 0, MMC_RSP_R2, 0, 0, NULL);
    if (ret) {
        printf("[MMC] %s: ALL_SEND_CID failed (%d)\n", __func__, ret);
        return ret;
    }
    printf("[MMC] %s: ALL_SEND_CID ok\n", __func__);

    ret = mmc_cmd(dev, MMC_SET_RELATIVE_ADDR, (uint32_t)dev->rca << 16, MMC_RSP_R1, 0, 0, NULL);
    if (ret) {
        printf("[MMC] %s: SET_RELATIVE_ADDR failed (%d)\n", __func__, ret);
        return ret;
    }

    printf("[MMC] %s: RCA=%u\n", __func__, dev->rca);

    ret = mmc_cmd(dev, MMC_SELECT_CARD, (uint32_t)dev->rca << 16, MMC_RSP_R1B, 0, 0, NULL);
    if (ret) {
        printf("[MMC] %s: SELECT_CARD failed (%d)\n", __func__, ret);
        return ret;
    }

    printf("[MMC] %s: card selected\n", __func__);

    ret = mmc_cmd(dev, MMC_SET_BLOCKLEN, MMC_BLOCK_SZ, MMC_RSP_R1, 0, 0, NULL);
    if (ret)
        printf("[MMC] %s: SET_BLOCKLEN warning (%d)\n", __func__, ret);

    memset(dev->ext_csd_local, 0, sizeof(dev->ext_csd_local));
    ret = mmc_read_ext_csd(dev, dev->ext_csd_local);
    if (ret) {
        printf("[MMC] %s: READ_EXT_CSD failed (%d), continuing\n", __func__, ret);
        dev->ext_csd = NULL;
    } else {
        printf("[MMC] %s: EXT_CSD rev=%u, sectors=%u, "
               "part_cfg=0x%02x, rpmb_mult=%u\n", __func__,
               dev->ext_csd_local[EXT_CSD_REV],
               *(uint32_t *)&dev->ext_csd_local[EXT_CSD_SEC_CNT],
               dev->ext_csd_local[EXT_CSD_PART_CONFIG],
               dev->ext_csd_local[EXT_CSD_RPMB_SIZE_MULT]);
    }

    printf("[MMC] %s: complete\n", __func__);
    return 0;
}

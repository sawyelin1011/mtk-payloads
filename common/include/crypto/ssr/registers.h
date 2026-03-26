/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: 2026 Shomy
 */

#pragma once

#include <stdint.h>
#include <mmio.h>

extern volatile uintptr_t ssr_base;

#define SSR_BASE            (ssr_base & ~0xFFFFUL)
#define SSR_TOP             (SSR_BASE + 0x5000)
#define SSR_CCC_BASE        (SSR_TOP)
#define SSR_RNG_BASE        (SSR_BASE + 0x1000)
#define SSR_KDF_BASE        (SSR_BASE + 0x3000)
#define SSR_PKA_BASE        (SSR_BASE + 0xA000)

#define SSR_LCS             (SSR_BASE + 0x18)

#define SSR_DOMAIN_REG(domain_base, off) ((domain_base) + (off))
#define CCC_REG(off)  SSR_DOMAIN_REG(SSR_CCC_BASE, (off))
#define KDF_REG(off)  SSR_DOMAIN_REG(SSR_KDF_BASE, (off))
#define PKA_REG(off)  SSR_DOMAIN_REG(SSR_PKA_BASE, (off))

#define SSR_CLK_CFG         CCC_REG(0x0000)
#define SSR_CLK_CFG_1       CCC_REG(0x0004)
#define SSR_CLK_CFG_2       CCC_REG(0x0008)

/// Clock

#define SSR_CLK_RNG         (1 << 0)
#define SSR_CLK_CCC         (1 << 8)
#define SSR_CLK_KDF         (1 << 16)
#define SSR_CLK_PKA         (1 << 24)

// Goya (MT6899) uses different registers
// than MT6878 or MT6989.
// Instead of 0xC, it uses 0x8, and instead of
// 0x114-0x118, it uses 0x104-0x108
//
// Furthermore, the implementation for SetClkRate
// differs quite a lot.
//
// TODO: Change these offsets.
#define TOPCKGEN_BASE               (0x10000000)
#define CLK_CFG_UPDATE2             (TOPCKGEN_BASE + 0x000C)
#define CLK_CFG_16_SET              (TOPCKGEN_BASE + 0x0114)
#define CLK_CFG_16_CLR              (TOPCKGEN_BASE + 0x0118)
#define CLK_CFG_17_SET              (TOPCKGEN_BASE + 0x0124)
#define CLK_CFG_17_CLR              (TOPCKGEN_BASE + 0x0128)

/// TOP

#define CCC_QUEUE                   CCC_REG(0x0104)
#define CCC_HW_INIT_CFG0            CCC_REG(0x02AC)
#define CCC_HW_INIT_CFG1            CCC_REG(0x02B0)
#define CCC_SKIP_KS_INIT            CCC_REG(0x02D4)
#define SSR_BOOT                    CCC_REG(0x02F8)
#define SSR_CLK_GATE                0x100

#define SSR_INIT_MAGIC1             0x35003400
#define SSR_INIT_MAGIC2             0x06bf3701

#define SSR_TIMEOUT                 (5000)

/// KDF

typedef enum {
    KEY_NULL = -1,
    SW_KEY = 0,
    INT_REG0 = 2,
    FUSE_KDR = 4,
    MAX = 5,
} SsrKdfKeySel;

typedef enum {
	AES_256 = 0,
	AES_128 = 1,
	AES_192 = 2,
} SsrAesType;

#define SSR_AES_KEY_SIZE(type) \
    ((type) == AES_256 ? (1 << 5) : \
     (type) == AES_128 ? (1 << 4) : \
     (type) == AES_192 ? ((1 << 3) | (1 << 4)) : 0)

#define SSR_KDF_AES_128             0x00000000
#define SSR_KDF_AES_192             0x00000010
#define SSR_KDF_AES_256             0x00000020

/* KDF CMAC */

#define SSR_KDF_CMAC_OUT0           KDF_REG(0x00C)  /* output bytes  0-3  */
#define SSR_KDF_CMAC_OUT1           KDF_REG(0x008)  /* output bytes  4-7  */
#define SSR_KDF_CMAC_OUT2           KDF_REG(0x004)  /* output bytes  8-11 */
#define SSR_KDF_CMAC_OUT3           KDF_REG(0x000)  /* output bytes 12-15 */
#define SSR_KDF_CMAC_OUT4           KDF_REG(0x01C)  /* output bytes 16-19 */
#define SSR_KDF_CMAC_OUT5           KDF_REG(0x018)  /* output bytes 20-23 */
#define SSR_KDF_CMAC_OUT6           KDF_REG(0x014)  /* output bytes 24-27 */
#define SSR_KDF_CMAC_OUT7           KDF_REG(0x010)  /* output bytes 28-31 */
#define SSR_KDF_CMAC_OUT8           KDF_REG(0x02C)  /* output bytes 32-35 */
#define SSR_KDF_CMAC_OUT9           KDF_REG(0x028)  /* output bytes 36-39 */
#define SSR_KDF_CMAC_OUT10          KDF_REG(0x024)  /* output bytes 40-43 */
#define SSR_KDF_CMAC_OUT11          KDF_REG(0x020)  /* output bytes 44-47 */
#define SSR_KDF_CMAC_OUT12          KDF_REG(0x03C)  /* output bytes 48-51 */
#define SSR_KDF_CMAC_OUT13          KDF_REG(0x038)  /* output bytes 52-55 */
#define SSR_KDF_CMAC_OUT14          KDF_REG(0x034)  /* output bytes 56-59 */
#define SSR_KDF_CMAC_OUT15          KDF_REG(0x030)  /* output bytes 60-63 */

#define SSR_KDF_CMAC_OUT(n)         KDF_REG(0x00C + ((n) / 4) * 0x10 - ((n) % 4) * 4)

#define SSR_KDF_CMAC_FIN16          KDF_REG(0x040)  /* SP800-108 input */
#define SSR_KDF_CMAC_FIN15          KDF_REG(0x044)
#define SSR_KDF_CMAC_FIN14          KDF_REG(0x048)
#define SSR_KDF_CMAC_FIN13          KDF_REG(0x04C)
#define SSR_KDF_CMAC_FIN12          KDF_REG(0x050)
#define SSR_KDF_CMAC_FIN11          KDF_REG(0x054)
#define SSR_KDF_CMAC_FIN10          KDF_REG(0x058)
#define SSR_KDF_CMAC_FIN9           KDF_REG(0x05C)
#define SSR_KDF_CMAC_FIN8           KDF_REG(0x060)
#define SSR_KDF_CMAC_FIN7           KDF_REG(0x064)
#define SSR_KDF_CMAC_FIN6           KDF_REG(0x068)
#define SSR_KDF_CMAC_FIN5           KDF_REG(0x06C)
#define SSR_KDF_CMAC_FIN4           KDF_REG(0x070)
#define SSR_KDF_CMAC_FIN3           KDF_REG(0x074)
#define SSR_KDF_CMAC_FIN2           KDF_REG(0x078)
#define SSR_KDF_CMAC_FIN1           KDF_REG(0x07C)
#define SSR_KDF_CMAC_FIN0           KDF_REG(0x080)  /* bits[7:0] = length */

#define SSR_KDF_CMAC_FIN(n)         KDF_REG(0x080 - (n) * 4)

#define SSR_KDF_CMAC_CMD            KDF_REG(0x084)
#define SSR_KDF_CMAC_STS            KDF_REG(0x08C)  /* Status, should be all zero for OK */

#define SSR_KDF_CMAC_LBL7           KDF_REG(0x0A0)  /* Label */
#define SSR_KDF_CMAC_LBL6           KDF_REG(0x0A4)
#define SSR_KDF_CMAC_LBL5           KDF_REG(0x0A8)
#define SSR_KDF_CMAC_LBL4           KDF_REG(0x0AC)
#define SSR_KDF_CMAC_LBL3           KDF_REG(0x0B0)
#define SSR_KDF_CMAC_LBL2           KDF_REG(0x0B4)
#define SSR_KDF_CMAC_LBL1           KDF_REG(0x0B8)
#define SSR_KDF_CMAC_LBL0           KDF_REG(0x0BC)

#define SSR_KDF_CMAC_ST             KDF_REG(0x0CC)  /* Self Test */

#define SSR_KDF_CMAC_FIN_WORDS      (17)
#define SSR_KDF_CMAC_FIN_MAX        (67)
#define SSR_KDF_CMAC_LBL_WORDS      (8)
#define SSR_KDF_CMAC_CMD_START      BIT(0)
#define SSR_KDF_CMAC_ST_DONE        BIT(31)

#define SSR_KDF_CMAC_CMD_KEY_ALG    GENMASK(7,  6)
#define SSR_KDF_CMAC_CMD_KEY_SRC    GENMASK(11, 8)
#define SSR_KDF_CMAC_CMD_OUT_BLOCKS GENMASK(20, 16)
#define SSR_KDF_CMAC_FIN_LEN        GENMASK(7, 0)
#define SSR_KDF_CMAC_STS_ERR        GENMASK(10, 0)
#define SSR_KDF_CMAC_ST_ERR         GENMASK(13, 0)

/* KDF HKDF (https://en.wikipedia.org/wiki/HKDF) */

#define SSR_KDF_HKDF_IKM15          KDF_REG(0x100)  /* Input Key Material */
#define SSR_KDF_HKDF_IKM14          KDF_REG(0x104)
#define SSR_KDF_HKDF_IKM13          KDF_REG(0x108)
#define SSR_KDF_HKDF_IKM12          KDF_REG(0x10C)
#define SSR_KDF_HKDF_IKM11          KDF_REG(0x110)
#define SSR_KDF_HKDF_IKM10          KDF_REG(0x114)
#define SSR_KDF_HKDF_IKM9           KDF_REG(0x118)
#define SSR_KDF_HKDF_IKM8           KDF_REG(0x11C)
#define SSR_KDF_HKDF_IKM7           KDF_REG(0x120)
#define SSR_KDF_HKDF_IKM6           KDF_REG(0x124)
#define SSR_KDF_HKDF_IKM5           KDF_REG(0x128)
#define SSR_KDF_HKDF_IKM4           KDF_REG(0x12C)
#define SSR_KDF_HKDF_IKM3           KDF_REG(0x130)
#define SSR_KDF_HKDF_IKM2           KDF_REG(0x134)
#define SSR_KDF_HKDF_IKM1           KDF_REG(0x138)
#define SSR_KDF_HKDF_IKM0           KDF_REG(0x13C)

#define SSR_KDF_HKDF_IKM(n)         KDF_REG(0x13C - (n) * 4)

#define SSR_KDF_HKDF_SALT7          KDF_REG(0x140)
#define SSR_KDF_HKDF_SALT6          KDF_REG(0x144)
#define SSR_KDF_HKDF_SALT5          KDF_REG(0x148)
#define SSR_KDF_HKDF_SALT4          KDF_REG(0x14C)
#define SSR_KDF_HKDF_SALT3          KDF_REG(0x150)
#define SSR_KDF_HKDF_SALT2          KDF_REG(0x154)
#define SSR_KDF_HKDF_SALT1          KDF_REG(0x158)
#define SSR_KDF_HKDF_SALT0          KDF_REG(0x15C)

#define SSR_KDF_HKDF_SALT(n)        KDF_REG(0x15C - (n) * 4)

#define SSR_KDF_HKDF_INFO11         KDF_REG(0x160)  /* Info, aka context*/
#define SSR_KDF_HKDF_INFO10         KDF_REG(0x164)
#define SSR_KDF_HKDF_INFO9          KDF_REG(0x168)
#define SSR_KDF_HKDF_INFO8          KDF_REG(0x16C)
#define SSR_KDF_HKDF_INFO7          KDF_REG(0x170)
#define SSR_KDF_HKDF_INFO6          KDF_REG(0x174)
#define SSR_KDF_HKDF_INFO5          KDF_REG(0x178)
#define SSR_KDF_HKDF_INFO4          KDF_REG(0x17C)
#define SSR_KDF_HKDF_INFO3          KDF_REG(0x180)
#define SSR_KDF_HKDF_INFO2          KDF_REG(0x184)
#define SSR_KDF_HKDF_INFO1          KDF_REG(0x188)
#define SSR_KDF_HKDF_INFO0          KDF_REG(0x18C)

#define SSR_KDF_HKDF_INFO(n)        KDF_REG(0x18C - (n) * 4)

#define SSR_KDF_HKDF_STS            KDF_REG(0x198)
#define SSR_KDF_HKDF_CMD            KDF_REG(0x19C)

#define SSR_KDF_HKDF_OUT0           KDF_REG(0x1FC)  /* output bytes  0-3  */
#define SSR_KDF_HKDF_OUT1           KDF_REG(0x1F8)  /* output bytes  4-7  */
#define SSR_KDF_HKDF_OUT2           KDF_REG(0x1F4)  /* output bytes  8-11 */
#define SSR_KDF_HKDF_OUT3           KDF_REG(0x1F0)  /* output bytes 12-15 */
#define SSR_KDF_HKDF_OUT4           KDF_REG(0x1EC)  /* output bytes 16-19 */
#define SSR_KDF_HKDF_OUT5           KDF_REG(0x1E8)  /* output bytes 20-23 */
#define SSR_KDF_HKDF_OUT6           KDF_REG(0x1E4)  /* output bytes 24-27 */
#define SSR_KDF_HKDF_OUT7           KDF_REG(0x1E0)  /* output bytes 28-31 */
#define SSR_KDF_HKDF_OUT8           KDF_REG(0x1DC)  /* output bytes 32-35 */
#define SSR_KDF_HKDF_OUT9           KDF_REG(0x1D8)  /* output bytes 36-39 */
#define SSR_KDF_HKDF_OUT10          KDF_REG(0x1D4)  /* output bytes 40-43 */
#define SSR_KDF_HKDF_OUT11          KDF_REG(0x1D0)  /* output bytes 44-47 */
#define SSR_KDF_HKDF_OUT12          KDF_REG(0x1CC)  /* output bytes 48-51 */
#define SSR_KDF_HKDF_OUT13          KDF_REG(0x1C8)  /* output bytes 52-55 */
#define SSR_KDF_HKDF_OUT14          KDF_REG(0x1C4)  /* output bytes 56-59 */
#define SSR_KDF_HKDF_OUT15          KDF_REG(0x1C0)  /* output bytes 60-63 */

#define SSR_KDF_HKDF_OUT(n)         KDF_REG(0x1FC - (n) * 4)

#define SSR_KDF_HKDF_IKM_WORDS      (16)
#define SSR_KDF_HKDF_IKM_MAX        (64)
#define SSR_KDF_HKDF_SALT_WORDS     (8)
#define SSR_KDF_HKDF_SALT_MAX       (32)
#define SSR_KDF_HKDF_INFO_WORDS     (12)
#define SSR_KDF_HKDF_INFO_MAX       (48)
#define SSR_KDF_HKDF_OUT_MAX        (64)

#define SSR_KDF_HKDF_CMD_START      BIT(0)
#define SSR_KDF_HKDF_CMD_FIXED      (BIT(5) | BIT(1)) // TODO: Figure out what these bits mean
#define SSR_KDF_HKDF_CMD_NO_SALT    BIT(3)
#define SSR_KDF_HKDF_CMD_IKM_LEN    GENMASK(15, 8)
#define SSR_KDF_HKDF_CMD_SALT_LEN   GENMASK(23, 16)
#define SSR_KDF_HKDF_CMD_INFO_LEN   GENMASK(31, 24)
#define SSR_KDF_HKDF_STS_ERR        GENMASK(4, 0)


/// PKA

typedef enum {
    SSR_PKA_ECC_CURVE_NIST_P256 = 0,
    SSR_PKA_ECC_CURVE_NIST_P384 = 4,
} ssr_pka_ecc_curve;

typedef enum {
    SSR_PKA_RSA_MODE_NULL = -1,
    SSR_PKA_RSA_MODE_1024, // 0
    SSR_PKA_RSA_MODE_2048, // 1
    SSR_PKA_RSA_MODE_3072, // 2
    SSR_PKA_RSA_MODE_4096, // 3
    SSR_PKA_RSA_MODE_MAX,  // 4
} ssr_pka_rsa_mode;

#define SSR_PKA_CTRL                PKA_REG(0x000)
#define SSR_PKA_CFG                 PKA_REG(0x010)   /* Init magic?                     */
#define SSR_PKA_STATUS_MASK         PKA_REG(0x208)   /* Init magic 2, maybe a mask      */
#define SSR_PKA_OP_TYPE             PKA_REG(0x20C)   /* operation type                  */
#define SSR_PKA_DONE                PKA_REG(0x200)   /* 0 when the operation is done    */
#define SSR_PKA_RESULT_ACK          PKA_REG(0x204)
#define SSR_PKA_START               PKA_REG(0x100)
#define SSR_PKA_RSA_KEY_IDX         PKA_REG(0x26C)
#define SSR_PKA_RSA_KEY_ZERO        PKA_REG(0x270)
#define SSR_PKA_ECC_CURVE           PKA_REG(0x2B8)   /* curve type */
#define SSR_PKA_ECC_OP_FIFO         PKA_REG(0x2CC)
#define SSR_PKA_ECC_DOM_FIFO        PKA_REG(0x2D0)
#define SSR_PKA_ECC_OP_B_FIFO       PKA_REG(0x2D4)
#define SSR_PKA_ECC_OP_C_FIFO       PKA_REG(0x2D8)
#define SSR_PKA_ECC_OP_D_FIFO       PKA_REG(0x2DC)
#define SSR_PKA_ECC_OP_E_FIFO       PKA_REG(0x2F8)
#define SSR_PKA_OP_A                PKA_REG(0x400)
#define SSR_PKA_OP_B                PKA_REG(0x800)
#define SSR_PKA_OP_C                PKA_REG(0xC00)  /* message */

#define SSR_PKA_ECC_P256_WORDS      (8)
#define SSR_PKA_ECC_P384_WORDS      (12)

#define SSR_PKA_OP_RSA_MODEXP       (2)
#define SSR_PKA_OP_ECC_GENKEY       (5)
#define SSR_PKA_OP_ECC_SIGN_P256    (6)
#define SSR_PKA_OP_ECC_SIGN_P384    (7)
#define SSR_PKA_OP_ECC_VERIFY_P256  (8)
#define SSR_PKA_OP_ECC_VERIFY_P384  (9)

#define PKA_RSA_SIGN_TIMEOUT        (10000)
#define PKA_RSA_VERIFY_TIMEOUT      (5000)

/// CCC

typedef union {
    uint8_t data_u8[64];
    uint32_t data_u32[16];
} SSR_SHA256_BUF_U;

typedef struct {
    SSR_SHA256_BUF_U buf;
    uintptr_t dma_base_address;
    uint32_t hash_val[8];
    uint64_t data_len; // ??
    uint8_t reserved[10];
} ssr_sha256_ctx;

typedef union {
    uint8_t data_u8[128];
    uint32_t data_u32[32];
} SSR_SHA384_BUF_U;

typedef struct {
    SSR_SHA384_BUF_U buf;
    uintptr_t dma_base_address;
    uint32_t hash_val[12];
    uint64_t data_len; // ??
    uint8_t reserved[10];
} ssr_sha384_ctx;

#define CCC_SHA_JOB_ID              CCC_REG(0x02C)
#define CCC_SHA_OUT(n)              CCC_REG(0x034 + (n) * 4)
#define CCC_QUEUE_AVAILABLE         CCC_REG(0x100)

#define CCC_SHA256_TYPE             (0x00)
#define CCC_SHA384_TYPE             (0x06)

/// ERRORS

#define SSR_OK                      0x0000
#define SSR_ERR_INVALID_PARAMETER   0x7245
#define SSR_ERR_INVALID_RANGE       0x7256
#define SSR_ERR_TIMEOUT             0x7249

#define SSR_KDF_ERR_TIMEOUT         0x7262
#define SSR_KDF_ERR_SELFTEST_FAIL   0x7268
#define SSR_KDF_ERR_NULL_PTR        0x7247
#define SSR_KDF_ERR_BAD_SIZE        0x7246
#define SSR_KDF_ERR_BAD_CONFIG      0x724C
#define SSR_KDF_ERR_HW_UNKNOWN      0x724B

#define SSR_PKA_TIMEOUT             0x7241
#define SSR_PKA_MODE_OUT_OF_RANGE   0x7242
#define SSR_PKA_ECC_INVALID_CURVE   0x725A

#define SSR_CCC_TIMEOUT             0x7275
#define SSR_CCC_ERROR               0x724E

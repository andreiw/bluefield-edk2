/** @file
  Header for the MMC Host Protocol implementation for the Designware controller.

  Copyright (c) 2016, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __DWMCI_H__
#define __DWMCI_H__

#include <Uefi.h>

#include <Protocol/MmcHost.h>

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>

#define DWMCI_DXE_VERSION 0x01

// MMC commands
#define MMC_GO_IDLE_STATE                0
#define MMC_SEND_OP_COND                 1
#define MMC_ALL_SEND_CID                 2
#define MMC_SET_RELATIVE_ADDR            3
#define MMC_SET_DSR                      4
#define MMC_SLEEP_AWAKE                  5
#define MMC_SWITCH                       6
#define MMC_SELECT_CARD                  7
#define MMC_SEND_EXT_CSD                 8
#define MMC_SEND_CSD                     9
#define MMC_SEND_CID                    10
#define MMC_READ_DAT_UNTIL_STOP         11
#define MMC_STOP_TRANSMISSION           12
#define MMC_SEND_STATUS                 13
#define MMC_BUS_TEST_R                  14
#define MMC_GO_INACTIVE_STATE           15
#define MMC_BUS_TEST_W                  19
#define MMC_SET_BLOCKLEN                16
#define MMC_READ_SINGLE_BLOCK           17
#define MMC_READ_MULTIPLE_BLOCK         18
#define MMC_SEND_TUNING_BLOCK           19
#define MMC_WRITE_DAT_UNTIL_STOP        20
#define MMC_SEND_TUNING_BLOCK_HS200     21
#define MMC_SET_BLOCK_COUNT             23
#define MMC_WRITE_BLOCK                 24
#define MMC_WRITE_MULTIPLE_BLOCK        25
#define MMC_PROGRAM_CID                 26
#define MMC_PROGRAM_CSD                 27
#define MMC_SET_WRITE_PROT              28
#define MMC_CLR_WRITE_PROT              29
#define MMC_SEND_WRITE_PROT             30
#define MMC_ERASE_GROUP_START           35
#define MMC_ERASE_GROUP_END             36
#define MMC_ERASE                       38
#define MMC_FAST_IO                     39
#define MMC_GO_IRQ_STATE                40
#define MMC_LOCK_UNLOCK                 42
#define MMC_APP_CMD                     55
#define MMC_GEN_CMD                     56
#define MMC_SPI_READ_OCR                58
#define MMC_SPI_CRC_ON_OFF              59

#define DW_SYSCTL FixedPcdGet64 (PcdDWMciBase)

#define DW_CTRL_REG           (DW_SYSCTL + 0x00)
#define DW_PWREN_REG          (DW_SYSCTL + 0x04)
#define DW_CLKDIV_REG         (DW_SYSCTL + 0x08)
#define DW_CLKSRC_REG         (DW_SYSCTL + 0x0c)
#define DW_CLKENA_REG         (DW_SYSCTL + 0x10)
#define DW_TMOUT_REG          (DW_SYSCTL + 0x14)
#define DW_CTYPE_REG          (DW_SYSCTL + 0x18)
#define DW_BLKSIZ_REG         (DW_SYSCTL + 0x1c)
#define DW_BYTCNT_REG         (DW_SYSCTL + 0x20)
#define DW_INTMASK_REG        (DW_SYSCTL + 0x24)
#define DW_CMDARG_REG         (DW_SYSCTL + 0x28)
#define DW_CMD_REG            (DW_SYSCTL + 0x2c)
#define DW_RESP0_REG          (DW_SYSCTL + 0x30)
#define DW_RESP1_REG          (DW_SYSCTL + 0x34)
#define DW_RESP2_REG          (DW_SYSCTL + 0x38)
#define DW_RESP3_REG          (DW_SYSCTL + 0x3c)
#define DW_MINTSTS_REG        (DW_SYSCTL + 0x40)
#define DW_RINTSTS_REG        (DW_SYSCTL + 0x44)
#define DW_STATUS_REG         (DW_SYSCTL + 0x48)
#define DW_FIFOTH_REG         (DW_SYSCTL + 0x4c)
#define DW_CDETECT_REG        (DW_SYSCTL + 0x50)
#define DW_WRTPRT_REG         (DW_SYSCTL + 0x54)
#define DW_GPIO_REG           (DW_SYSCTL + 0x58)
#define DW_TCBCNT_REG         (DW_SYSCTL + 0x5c)
#define DW_TBBCNT_REG         (DW_SYSCTL + 0x60)
#define DW_DEBNCE_REG         (DW_SYSCTL + 0x64)
#define DW_USRID_REG          (DW_SYSCTL + 0x68)
#define DW_VERID_REG          (DW_SYSCTL + 0x6c)
#define DW_HCON_REG           (DW_SYSCTL + 0x70)
#define DW_UHS_REG_REG        (DW_SYSCTL + 0x74)
#define DW_RST_N_REG          (DW_SYSCTL + 0x78)
#define DW_BMOD_REG           (DW_SYSCTL + 0x80)
#define DW_PLDMND_REG         (DW_SYSCTL + 0x84)

/* 32-bit flavors */
#define DW_DBADDR_REG         (DW_SYSCTL + 0x88)
#define DW_IDSTS_REG          (DW_SYSCTL + 0x8c)
#define DW_IDINTEN_REG        (DW_SYSCTL + 0x90)
#define DW_DSCADDR_REG        (DW_SYSCTL + 0x94)
#define DW_BUFADDR_REG        (DW_SYSCTL + 0x98)
/* 64-bit flavors */
#define DW_DBADDRL_REG        (DW_SYSCTL + 0x88)
#define DW_DBADDRU_REG        (DW_SYSCTL + 0x8c)
#define DW_IDSTS64_REG        (DW_SYSCTL + 0x90)
#define DW_IDINTEN64_REG      (DW_SYSCTL + 0x94)
#define DW_DSCADDRL_REG       (DW_SYSCTL + 0x98)
#define DW_DSCADDRU_REG       (DW_SYSCTL + 0x9c)
#define DW_BUFADDRL_REG       (DW_SYSCTL + 0xa0)
#define DW_BUFADDRU_REG       (DW_SYSCTL + 0xa4)

#define DW_CARDTHRCTL_REG     (DW_SYSCTL + 0x100)
#define DW_BACK_END_POWER_REG (DW_SYSCTL + 0x104)
#define DW_UHS_REG_EXT_REG    (DW_SYSCTL + 0x108)
#define DW_EMMC_DDR_REG_REG   (DW_SYSCTL + 0x10c)
#define DW_ENABLE_SHIFT_REG   (DW_SYSCTL + 0x110)
#define DW_FIFO_REG           (DW_SYSCTL + 0x200)

#define DW_CTRL_CONTROLLER_RESET     BIT0
#define DW_CTRL_FIFO_RESET           BIT1
#define DW_CTRL_DMA_RESET            BIT2
#define DW_CTRL_INT_ENABLE           BIT4
#define DW_CTRL_DMA_ENABLE           BIT5
#define DW_CTRL_READ_WAIT            BIT6
#define DW_CTRL_SEND_IRQ_RESPONSE    BIT7
#define DW_CTRL_ABORT_READ_DATA      BIT8
#define DW_CTRL_SEND_CSSD            BIT9
#define DW_CTRL_SEND_AUTO_STOP_CSSD  BIT10
#define DW_CTRL_INTERRUPT_STATUS     BIT11
#define DW_CTRL_ENABLE_OD_PULLUP     BIT24
#define DW_CTRL_USE_INTERNAL_DMAC    BIT25

#define DW_CMD_INDEX                 0x3F
#define DW_CMD_EXPECT_RESPONSE       BIT6
#define DW_CMD_LONG_RESPONSE         BIT7
#define DW_CMD_CHECK_RESP_CRC        BIT8
#define DW_CMD_DATA_EXPECTED         BIT9
#define DW_CMD_READ_WRITE            BIT10
#define DW_CMD_TRANSFER_MODE         BIT11
#define DW_CMD_SEND_AUTO_STOP        BIT12
#define DW_WAIT_PRVDATA_COMPLETE     BIT13
#define DW_CMD_STOP_ABORT_CMD        BIT14
#define DW_CMD_SEND_INIT             BIT15
#define DW_CMD_UPDATE_CLK_ONLY       BIT21
#define DW_CMD_READ_CAEDA_DEVICE     BIT22
#define DW_CMD_CCS_EXPECTED          BIT23
#define DW_CMD_ENABLE_BOOT           BIT24
#define DW_CMD_EXPECT_BOOK_ACK       BIT25
#define DW_CMD_DISABLE_BOOT          BIT26
#define DW_CMD_BOOT_MODE             BIT27
#define DW_CMD_VOLT_SWITCH           BIT28
#define DW_CMD_USE_HOLD_REG          BIT29
#define DW_CMD_START_CMD             BIT31

#define DW_INT_RE   BIT1  // Response error
#define DW_INT_CDO  BIT2  // Command done
#define DW_INT_DTO  BIT3  // Data transfer over
#define DW_INT_RCRC BIT6  // Response CRC error
#define DW_INT_DCRC BIT7  // Data CRC error
#define DW_INT_RTO  BIT8  // Response timeout
#define DW_INT_DRTO BIT9  // Data read timeout
#define DW_INT_HLE  BIT12 // Hardware locked write error
#define DW_INT_SBE  BIT13 // Start Bit Error (SBE) / Busy Complete Interrupt
#define DW_INT_EBE  BIT15 // End-bit error (read) / Write no CRC

#define DW_INT_CMD_ERRORS \
  (DW_INT_RE | DW_INT_RCRC | DW_INT_RTO | DW_INT_HLE)

#define DW_INT_READ_ERRORS \
  (DW_INT_DCRC | DW_INT_DRTO | DW_INT_SBE | DW_INT_EBE)

#define DW_INT_WRITE_ERRORS \
  (DW_INT_DCRC | DW_INT_DRTO | DW_INT_EBE)

#define DW_STATUS_DATA_BUSY          BIT9
#define DW_STATUS_DATA_STATE_MC_BUSY BIT10
#define DW_STATUS_FIFO_COUNT(s) (((s) >> 17) & 0x1fff)

#define DWMCI_TRACE(txt)                DEBUG ((EFI_D_BLKIO, "DWMCI: " txt "\n"))

#endif /* __DWMCI_H__ */

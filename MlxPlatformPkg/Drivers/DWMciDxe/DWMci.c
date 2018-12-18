/** @file
  This file implements the MMC Host Protocol for the Designware
  controller.

  Copyright (c) 2016, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials are licensed and made
  available under the terms and conditions of the BSD License which
  accompanies this distribution.  The full text of the license may be
  found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
  EXPRESS OR IMPLIED.

**/

#include "DWMci.h"
#include "Simulator.h"
#include "rsh_def.h"
#include "BlueFieldPlatform.h"
#include "BlueFieldEfiInfo.h"

#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>

// Use card 0 in UEFI. This is the only card supported in UEFI for now.
STATIC int cardId = 0;

// FIXME: We should detect the FIFO DEPTH dynamically.
#define DWMCI_FIFO_DEPTH        128
// This corresponds to about 0.01 ms.
#define DWMCI_TIMEOUT          1000

#define DWMCI_CLK              50000000
#define DWMCI_CLK_400KHZ_DIVIDER (DWMCI_CLK / (400000 * 2))
#define DWMCI_CLK_24MHZ_DIVIDER  (DWMCI_CLK / (24000000 * 2))

BOOLEAN
DWMciIsCardPresent (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  // FIXME: This might eventually query some GPIO pin which currently
  // does not exist.
  return TRUE;
}

BOOLEAN
DWMciIsReadOnly (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  // FIXME: This might eventually query some GPIO pin which currently
  // does not exist.
  return FALSE;
}

#if !defined(MDEPKG_NDEBUG)
VOID
DWMciPrintRegs (
  )
{
  UINT32  Reg;

  Reg = MmioRead32 (DW_CTRL_REG);
  DEBUG ((EFI_D_ERROR, "Ctrl    0x%x\n", Reg));

  Reg = MmioRead32 (DW_PWREN_REG);
  DEBUG ((EFI_D_ERROR, "Pwren   0x%x\n", Reg));

  Reg = MmioRead32 (DW_CLKDIV_REG);
  DEBUG ((EFI_D_ERROR, "Clkdiv  0x%x\n", Reg));

  Reg = MmioRead32 (DW_CLKSRC_REG);
  DEBUG ((EFI_D_ERROR, "Clksrc  0x%x\n", Reg));

  Reg = MmioRead32 (DW_CLKENA_REG);
  DEBUG ((EFI_D_ERROR, "Clkena  0x%x\n", Reg));

  Reg = MmioRead32 (DW_TMOUT_REG);
  DEBUG ((EFI_D_ERROR, "Tmout   0x%x\n", Reg));

  Reg = MmioRead32 (DW_INTMASK_REG);
  DEBUG ((EFI_D_ERROR, "Intmask 0x%x\n", Reg));

  Reg = MmioRead32 (DW_CTYPE_REG);
  DEBUG ((EFI_D_ERROR, "Ctype   0x%x\n", Reg));

  Reg = MmioRead32 (DW_BLKSIZ_REG);
  DEBUG ((EFI_D_ERROR, "Blksize 0x%x\n", Reg));

  Reg = MmioRead32 (DW_BYTCNT_REG);
  DEBUG ((EFI_D_ERROR, "Bytcnt  0x%x\n", Reg));

  Reg = MmioRead32 (DW_RINTSTS_REG);
  DEBUG ((EFI_D_ERROR, "Rintsts 0x%x\n", Reg));

  Reg = MmioRead32 (DW_STATUS_REG);
  DEBUG ((EFI_D_ERROR, "Status  0x%x\n", Reg));

  Reg = MmioRead32 (DW_FIFOTH_REG);
  DEBUG ((EFI_D_ERROR, "Fifoth  0x%x\n", Reg));

  Reg = MmioRead32 (DW_CDETECT_REG);
  DEBUG ((EFI_D_ERROR, "Cdetect 0x%x\n", Reg));

  Reg = MmioRead32 (DW_WRTPRT_REG);
  DEBUG ((EFI_D_ERROR, "Wrtprt  0x%x\n", Reg));

  Reg = MmioRead32 (DW_VERID_REG);
  DEBUG ((EFI_D_ERROR, "Verid   0x%x\n", Reg));

  Reg = MmioRead32 (DW_HCON_REG);
  DEBUG ((EFI_D_ERROR, "Hcon    0x%x\n", Reg));

  Reg = MmioRead32 (DW_UHS_REG_REG);
  DEBUG ((EFI_D_ERROR, "Uhs     0x%x\n", Reg));

  Reg = MmioRead32 (DW_RST_N_REG);
  DEBUG ((EFI_D_ERROR, "Rst_n   0x%x\n", Reg));

  Reg = MmioRead32 (DW_UHS_REG_EXT_REG);
  DEBUG ((EFI_D_ERROR, "Uhs_ext 0x%x\n", Reg));
}
#endif

void DWMciSendCmdAndWait(
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  )
{
  MmcCmd |= DW_CMD_USE_HOLD_REG;

  // Wait until card is not busy.
  while (MmioRead32 (DW_STATUS_REG) & DW_STATUS_DATA_BUSY) {
    MicroSecondDelay (10);
  }

  // Set the command argument.
  MmioWrite32 (DW_CMDARG_REG, Argument);

  // Add a memory fense to make sure argument is ready before command.
  MemoryFence ();

  // Send the command.
  MmioWrite32 (DW_CMD_REG, MmcCmd | (cardId << 16));

  MemoryFence ();

  // Wait until the Start bit is cleared.
  while (MmioRead32 (DW_CMD_REG) & DW_CMD_START_CMD) {
    MicroSecondDelay (10);
  }
}

STATIC
VOID
DWMciCtrlReset(
  IN UINT32                     Reset
)
{
  UINT32  Ctl;

  Ctl = MmioRead32 (DW_CTRL_REG);
  MmioWrite32 (DW_CTRL_REG, Ctl | Reset);
  do {
    Ctl = MmioRead32 (DW_CTRL_REG);
  } while (Ctl & Reset);
}

STATIC
EFI_STATUS
DWMciInitializeController (
  )
{
  BOOLEAN        BootFromEmmc;
  MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

  DWMCI_TRACE ("Initializing DW MMC controller");

  // When booting from eMMC, HW will send commands to the cards via the CMD
  // register.
  BootFromEmmc = (MmioRead32 (DW_CMD_REG) &
    (DW_CMD_DATA_EXPECTED | DW_CMD_BOOT_MODE)) ? TRUE : FALSE;
  Info->ExtBoot = !BootFromEmmc;

  // Reset DMA / FIFOs / Controller.
  DWMciCtrlReset (DW_CTRL_FIFO_RESET | DW_CTRL_DMA_RESET |
    DW_CTRL_CONTROLLER_RESET);

  // SW reset IDMAC (Internal Direct Memory Access Controller).
  MmioWrite32 (DW_BMOD_REG, 0x1);

  // Set the timeout register.
  MmioWrite32 (DW_TMOUT_REG, 0xFFFFFFFF);

  // Set FIFO threshold. FIFO size is 0x100 on the SoC.
  // So set Tx threshold 0x80 and Rx threshold 0x7f as suggested by the DWC
  // spec. Set single transfer since DMA is not used here.
  MmioWrite32 (DW_FIFOTH_REG, 0x007f0080);

  // Add memory fence to drain the above write.
  MemoryFence ();

  // Clear all interrupt status bits.
  MmioWrite32 (DW_RINTSTS_REG, 0xFFFFFFFF);

  // Disable each individual interrupt.
  MmioWrite32 (DW_INTMASK_REG, 0);

  // Enable the global interrupt.
  MmioWrite32 (DW_CTRL_REG, MmioRead32 (DW_CTRL_REG) | DW_CTRL_INT_ENABLE);

  // Add memory fence to drain the above write.
  MemoryFence ();

  // Disable clock.
  MmioWrite32 (DW_CLKENA_REG, 0);
  DWMciSendCmdAndWait (DW_CMD_START_CMD | DW_WAIT_PRVDATA_COMPLETE |
    DW_CMD_UPDATE_CLK_ONLY, 0);

  // Update CTYPE/CLKDIV/CLKSRC according to the spec. Clock 50MHz divided
  // by (2 * 60) will be ~400KHz which is the required clock rate
  // during initialization phase according to the spec.
  // When booting from eMMC, the bus should be set to 8-bit mode in the CTYPE
  // register since this is the mode HW will use. For external boot, the CTYPE
  // register should be 0 (default 1-bit mode).
  if (Info->BfIsPal) {
    MmioWrite32 (DW_CTYPE_REG, 0);
  } else {
    MmioWrite32 (DW_CTYPE_REG, BootFromEmmc ? (0x10000 << cardId) : 0);
  }
  MmioWrite32 (DW_CLKSRC_REG, 0);
  MmioWrite32 (DW_CLKDIV_REG, DWMCI_CLK_400KHZ_DIVIDER);

  // Re-enable the clock.
  MmioWrite32 (DW_CLKENA_REG, 0x10001 << cardId);
  DWMciSendCmdAndWait (DW_CMD_START_CMD | DW_WAIT_PRVDATA_COMPLETE |
    DW_CMD_UPDATE_CLK_ONLY, 0);

  // Send the initialization sequence (the DW_CMD_SEND_INIT flag).
  DWMciSendCmdAndWait (MMC_GO_IDLE_STATE | DW_CMD_START_CMD |
    DW_CMD_STOP_ABORT_CMD | DW_CMD_DISABLE_BOOT | DW_CMD_SEND_INIT, 0);

  // Set the default UHS-1 register (High Voltage + Non-DDR mode).
  MmioWrite32 (DW_UHS_REG_REG, 0);

  if (!Info->BfIsPal) {
    // Set drive=4 (bit 29:23) and sample=2 (bit 22:16) in UHS_REG_EXT.
    MmioWrite32 (DW_UHS_REG_EXT_REG, 0x2020000);
  }

  // Set the block size.
  MmioWrite32 (DW_BLKSIZ_REG, 0x200);

  // Enable the card read threshold.
  MmioWrite32 (DW_CARDTHRCTL_REG, 0x2000001);

  // Reset BYTCNT. I don't know if this is necessary but coming out
  // of boot this value is often out of wack.
  MmioWrite32 (DW_BYTCNT_REG, 0);

  return EFI_SUCCESS;
}

VOID
DWMciPrepareDataPath (
  )
{
  // Set Data Timer
  MmioWrite32 (DW_TMOUT_REG, 0xFFFFFFFF);

  // Set byte count to one block
  MmioWrite32 (DW_BYTCNT_REG, 512);
}

VOID
DwMciUpdateSwitchCmd (
  IN UINT32   Argument)
{
  UINT8 Access;
  UINT8 Index;
  UINT8 Value;

  Access = (Argument >> MMC_SWITCH_ACCESS_OFFSET) & MMC_SWITCH_ACCESS_MASK;
  Index = (Argument >> MMC_SWITCH_INDEX_OFFSET) & MMC_SWITCH_INDEX_MASK;
  Value = (Argument >> MMC_SWITCH_VALUE_OFFSET) & MMC_SWITCH_VALUE_MASK;

  // If we are changing bus width, we need to update DW_CTYPE_REG
  // accordingly.  FIXME: need to support multiple cards!
  if (Access == MMC_SWITCH_ACCESS_WRITE_BYTE &&
      Index == EMMC_ECSD_BUS_WIDTH_OFFSET) {

    // Disable clock.
    MmioWrite32 (DW_CLKENA_REG, 0);
    DWMciSendCmdAndWait (DW_CMD_START_CMD | DW_WAIT_PRVDATA_COMPLETE |
      DW_CMD_UPDATE_CLK_ONLY, 0);

    switch (Value) {
    case EMMC_ECSD_BUS_WIDTH_1_BIT:
      MmioWrite32 (DW_CTYPE_REG, 0 << cardId);
      // Set clock divider register to get 400KHz clock at 1-bit mode which
      // is usually initialization phase.
      MmioWrite32 (DW_CLKDIV_REG, DWMCI_CLK_400KHZ_DIVIDER);
      break;
    case EMMC_ECSD_BUS_WIDTH_4_BIT:
      MmioWrite32 (DW_CTYPE_REG, 1 << cardId);
      // Set clock divider register to get 24MHz (desired) clock.
      MmioWrite32 (DW_CLKDIV_REG, DWMCI_CLK_24MHZ_DIVIDER);
      break;
    case EMMC_ECSD_BUS_WIDTH_8_BIT:
      MmioWrite32 (DW_CTYPE_REG, 0x10000 << cardId);
      // Set clock divider register to get 24MHz (desired) clock.
      MmioWrite32 (DW_CLKDIV_REG, DWMCI_CLK_24MHZ_DIVIDER);
      break;
    }

    // Enable the clock.
    MmioWrite32 (DW_CLKENA_REG, 0x00001 << cardId);
    DWMciSendCmdAndWait (DW_CMD_START_CMD | DW_WAIT_PRVDATA_COMPLETE |
      DW_CMD_UPDATE_CLK_ONLY, 0);
  }
}

EFI_STATUS
DWMciSendCommand (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  )
{
  UINT32  IntStatus;
  UINT32  CmdIndex;
  UINT32  Cmd;
  UINTN   Timer;
  UINTN   RetVal;
  UINT32  Status;

  CmdIndex = MMC_GET_INDX (MmcCmd) & DW_CMD_INDEX;

  // We do not support multi-block reads or writes.
  if (CmdIndex == MMC_READ_MULTIPLE_BLOCK ||
      CmdIndex == MMC_WRITE_MULTIPLE_BLOCK) {
    return EFI_INVALID_PARAMETER;
  }

  if (CmdIndex == MMC_SEND_EXT_CSD ||
      CmdIndex == MMC_READ_SINGLE_BLOCK ||
      CmdIndex == MMC_WRITE_BLOCK) {
    DWMciPrepareDataPath ();
  }

  // Create controller command
  Cmd = CmdIndex | DW_CMD_START_CMD;

  // Except for send status command or commands that stop
  // transmission, wait for previous data command to complete.
  if (CmdIndex != MMC_GO_IDLE_STATE &&
      CmdIndex != MMC_STOP_TRANSMISSION &&
      CmdIndex != MMC_SEND_STATUS &&
      CmdIndex != MMC_GO_INACTIVE_STATE) {
    Cmd |= DW_WAIT_PRVDATA_COMPLETE;
  }

  // Set MmcCmd bits.
  if (MmcCmd & MMC_CMD_WAIT_RESPONSE) {
    Cmd |= DW_CMD_EXPECT_RESPONSE;
  }

  if (MmcCmd & MMC_CMD_LONG_RESPONSE) {
    Cmd |= DW_CMD_LONG_RESPONSE;
  }

  // Set command-specific bits.
  switch (CmdIndex) {
  case MMC_SEND_EXT_CSD:
  case MMC_READ_SINGLE_BLOCK:
    Cmd |= DW_CMD_DATA_EXPECTED;
    break;
  case MMC_WRITE_BLOCK:
    Cmd |= DW_CMD_DATA_EXPECTED | DW_CMD_READ_WRITE;
    break;
  }

  IntStatus = MmioRead32 (DW_RINTSTS_REG);
  MmioWrite32 (DW_RINTSTS_REG, IntStatus & (DW_INT_CDO | DW_INT_CMD_ERRORS));

  // Write to command and argument
  DWMciSendCmdAndWait (Cmd, Argument);

  // Wait for hardware-locked error or command done status.
  do {
    IntStatus = MmioRead32 (DW_RINTSTS_REG);
  } while (!(IntStatus & (DW_INT_CDO | DW_INT_HLE)));

  // Detect errror and set RetVal.
  if (IntStatus & DW_INT_CMD_ERRORS) {
    DEBUG ((EFI_D_ERROR, "DWMciSendCommand(0x%x) error:", Cmd));
    if (IntStatus & DW_INT_RE)
      DEBUG ((EFI_D_ERROR, " RE"));
    if (IntStatus & DW_INT_RCRC)
      DEBUG ((EFI_D_ERROR, " RCRC"));
    if (IntStatus & DW_INT_RTO)
      DEBUG ((EFI_D_ERROR, " RTO"));
    if (IntStatus & DW_INT_HLE)
      DEBUG ((EFI_D_ERROR, " HLE"));
    DEBUG ((EFI_D_ERROR, "\n"));
    RetVal = EFI_DEVICE_ERROR;
  }
  else {
    RetVal = EFI_SUCCESS;
  }

  // Clear interrupts.
  MmioWrite32 (DW_RINTSTS_REG, IntStatus & (DW_INT_CDO | DW_INT_CMD_ERRORS));

  // The SWITCH command requires special handling.  It changes the
  // operating modes of the card (e.g. bus width) and we may need to
  // update the controller to keep the two consistent.
  if (CmdIndex == MMC_SWITCH) {
    // Wait until the card is not busy.
    Timer = DWMCI_TIMEOUT * 60;
    do {
      MicroSecondDelay (10);
      Status = MmioRead32 (DW_STATUS_REG);
      Timer--;
    } while (((Status & DW_STATUS_DATA_BUSY) != 0) && Timer > 0);

    if ((Status & DW_STATUS_DATA_BUSY) != 0) {
      return EFI_DEVICE_ERROR;
    }

    // Do any special controller update for this command.
    DwMciUpdateSwitchCmd (Argument);
  }

  return RetVal;
}

EFI_STATUS
DWMciReceiveResponse (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_RESPONSE_TYPE          Type,
  IN UINT32                    *Buffer
  )
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // There should be only two valid response types: 0 for short and 1
  // for long.
  ASSERT (Type == 0 || Type == 1);

  if (Type == 0) {
    Buffer[0] = MmioRead32 (DW_RESP0_REG);
  } else {
    Buffer[0] = MmioRead32 (DW_RESP0_REG);
    Buffer[1] = MmioRead32 (DW_RESP1_REG);
    Buffer[2] = MmioRead32 (DW_RESP2_REG);
    Buffer[3] = MmioRead32 (DW_RESP3_REG);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DWMciReadBlockData (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN EFI_LBA                    Lba,
  IN UINTN                      Length,
  IN UINT32                    *Buffer
  )
{
  UINTN I;
  UINTN Count;
  UINTN Read;
  UINTN Finish;
  UINT32 IntStatus;
  EFI_TPL Tpl;

  if (Length > DWMCI_FIFO_DEPTH * 4) {
    return EFI_INVALID_PARAMETER;
  }

  if (Length % 4 != 0) {
    return EFI_INVALID_PARAMETER;
  }

  // Raise the TPL to the highest level to disable interrupts.
  Tpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  // Read data from the RX FIFO
  Read = 0;
  Finish = Length / 4;
  IntStatus = 0;
  while (Read < Finish) {
    Count = DW_STATUS_FIFO_COUNT (MmioRead32 (DW_STATUS_REG));

    for (I = 0; (I < Count) && (Read < Finish); I++) {
      Buffer[Read++] = MmioRead32 (DW_FIFO_REG);
    }

    // Check for errors.
    IntStatus = MmioRead32 (DW_RINTSTS_REG);
    if (IntStatus & DW_INT_READ_ERRORS) {
      break;
    }
  }

  // Restore TPL
  gBS->RestoreTPL (Tpl);

  if (IntStatus & DW_INT_READ_ERRORS) {
    DEBUG ((EFI_D_ERROR, "DWMciReadBlockData() error: "));
    if (IntStatus & DW_INT_DCRC)
      DEBUG ((EFI_D_ERROR, " DCRC"));
    if (IntStatus & DW_INT_DRTO)
      DEBUG ((EFI_D_ERROR, " DRTO"));
    if (IntStatus & DW_INT_SBE)
      DEBUG ((EFI_D_ERROR, " SBE"));
    if (IntStatus & DW_INT_EBE)
      DEBUG ((EFI_D_ERROR, " EBE"));
    DEBUG ((EFI_D_ERROR, "\n"));

    // Clear interrupts.
    MmioWrite32 (DW_RINTSTS_REG, IntStatus & DW_INT_READ_ERRORS);

    if (IntStatus & ~DW_INT_DRTO) {
      return EFI_DEVICE_ERROR;
    }
    else {
      return EFI_TIMEOUT;
    }
  }
  else {
    return EFI_SUCCESS;
  }
}

EFI_STATUS
DWMciWriteBlockData (
  IN EFI_MMC_HOST_PROTOCOL    *This,
  IN EFI_LBA                   Lba,
  IN UINTN                     Length,
  IN UINT32                   *Buffer
  )
{
  UINTN I;
  UINTN Count;
  UINTN Written;
  UINTN Finish;
  UINTN Timer;
  UINT32 IntStatus;
  UINT32 Status;
  UINTN Remaining;
  EFI_TPL Tpl;

  if (Length > DWMCI_FIFO_DEPTH * 4) {
    return EFI_INVALID_PARAMETER;
  }

  if (Length % 4 != 0) {
    return EFI_INVALID_PARAMETER;
  }

  // Raise the TPL at the highest level to disable interrupts.
  Tpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);

  // Write the data to the TX FIFO
  Written = 0;
  Finish = Length / 4;
  IntStatus = 0;
  while (Written < Finish) {
    Count = (DWMCI_FIFO_DEPTH -
             DW_STATUS_FIFO_COUNT (MmioRead32 (DW_STATUS_REG)));

    for (I = 0; (I < Count) && (Written < Finish); I++) {
      MmioWrite32 (DW_FIFO_REG, Buffer[Written++]);
    }

    // Check for errors.
    IntStatus = MmioRead32 (DW_RINTSTS_REG);
    if (IntStatus & DW_INT_WRITE_ERRORS) {
      break;
    }
  }

  // Restore TPL
  gBS->RestoreTPL (Tpl);

  if (IntStatus & DW_INT_WRITE_ERRORS) {
    DEBUG ((EFI_D_ERROR, "DWMciWriteBlockData() error: "));
    if (IntStatus & DW_INT_DCRC) {
      DEBUG ((EFI_D_ERROR, " DCRC"));
    }
    if (IntStatus & DW_INT_DRTO) {
      DEBUG ((EFI_D_ERROR, " DRTO"));
    }
    if (IntStatus & DW_INT_EBE) {
      DEBUG ((EFI_D_ERROR, " EBE"));
    }
    DEBUG ((EFI_D_ERROR, "\n"));

    // Clear interrupts.
    MmioWrite32 (DW_RINTSTS_REG, IntStatus & DW_INT_WRITE_ERRORS);

    if (IntStatus & ~DW_INT_DRTO) {
      return EFI_DEVICE_ERROR;
    }
    else {
      return EFI_TIMEOUT;
    }
  }

  // Wait for FIFO to drain
  Timer = DWMCI_TIMEOUT * 60;
  Status = MmioRead32 (DW_STATUS_REG);
  while (DW_STATUS_FIFO_COUNT (Status) && Timer > 0) {
    NanoSecondDelay (10);
    Status = MmioRead32 (DW_STATUS_REG);
    Timer--;
  }

  Remaining = DW_STATUS_FIFO_COUNT (Status);
  if (Remaining) {
    DEBUG ((EFI_D_ERROR,
            "DWMciWriteBlockData() timeout: "
            "words written 0x%x remaining 0x%x\n", Written, Remaining));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DWMciNotifyState (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_STATE                  State
  )
{
  switch (State) {
  case MmcHwInitializationState:
    DWMciInitializeController();
    break;
  case MmcIdleState:
  case MmcReadyState:
  case MmcIdentificationState:
  case MmcStandByState:
  case MmcTransferState:
  case MmcSendingDataState:
  case MmcReceiveDataState:
  case MmcProgrammingState:
  case MmcDisconnectState:
    break;
  case MmcInvalidState:
  default:
    ASSERT (0);
  }

  return EFI_SUCCESS;
}

EFI_GUID mDWMciDevicePathGuid = EFI_CALLER_ID_GUID;

EFI_STATUS
DWMciBuildDevicePath (
  IN EFI_MMC_HOST_PROTOCOL      *This,
  IN EFI_DEVICE_PATH_PROTOCOL  **DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *NewDevicePathNode;

  NewDevicePathNode = CreateDeviceNode (HARDWARE_DEVICE_PATH, HW_VENDOR_DP,
                                        sizeof (VENDOR_DEVICE_PATH));
  CopyGuid (&((VENDOR_DEVICE_PATH*)NewDevicePathNode)->Guid,
            &mDWMciDevicePathGuid);

  *DevicePath = NewDevicePathNode;
  return EFI_SUCCESS;
}

EFI_MMC_HOST_PROTOCOL gDWMciHost = {
  MMC_HOST_PROTOCOL_REVISION,
  DWMciIsCardPresent,
  DWMciIsReadOnly,
  DWMciBuildDevicePath,
  DWMciNotifyState,
  DWMciSendCommand,
  DWMciReceiveResponse,
  DWMciReadBlockData,
  DWMciWriteBlockData
};

EFI_STATUS
DWMciDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;
  EFI_HANDLE    Handle;

  Handle = NULL;

  // Check that the FIFO is big enough to handle default MMC block
  // size.
  ASSERT (DWMCI_FIFO_DEPTH >= 128);

  // Publish Component Name, BlockIO protocol interfaces
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiMmcHostProtocolGuid,
                  &gDWMciHost,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

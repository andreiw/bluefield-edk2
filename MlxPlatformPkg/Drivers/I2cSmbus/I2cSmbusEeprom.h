/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_SMBUS_EEPROM_H__
#define __I2C_SMBUS_EEPROM_H__

#include <Uefi.h>

#include <Protocol/SmbusHc.h>
#include <Protocol/Eeprom.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Pi/PiI2c.h>

#define EEPROM_SIGNATURE          SIGNATURE_32 ('E', 'E', 'P', 'R')

// Defines whether to use packet error code (PEC).
#define EEPROM_PEC_CHECK_EN         FALSE

// Encapsulates EEPROM context information.
typedef struct {
  UINT32                         Signature;
  EFI_HANDLE                     ControllerHandle;
  INTN                           Chip;
  UINT32                         Size;
  UINT8                          AddressWidth;
  UINT8                          PageSize;
  BOOLEAN                        PecEnable;
  EFI_SMBUS_HC_EXECUTE_OPERATION I2cSmbusExecute;
  BLUEFIELD_EEPROM_PROTOCOL      EepromProtocol;
} EEPROM_CONTEXT;

#define EEPROM_SC_FROM_EEPROM(a) \
    CR (a, EEPROM_CONTEXT, EepromProtocol, EEPROM_SIGNATURE)

extern EEPROM_CONTEXT gEepromContext;

EFI_STATUS
EFIAPI
I2cSmbusEepromTransfer (
  IN CONST BLUEFIELD_EEPROM_PROTOCOL *This,
  IN UINT16                          Address,
  IN UINT32                          Length,
  IN UINT8                           *Buffer,
  IN UINT8                           Operation
  );

EFI_STATUS
EFIAPI
I2cSmbusInstallEepromProtocol (
  VOID
  );

#endif // __I2C_SMBUS_EEPROM_H__

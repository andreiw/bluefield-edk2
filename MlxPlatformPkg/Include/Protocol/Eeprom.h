/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BLUEFIELD_EEPROM_H__
#define __BLUEFIELD_EEPROM_H__

#define BLUEFIELD_EEPROM_PROTOCOL_GUID { 0x71954bda, 0x60d3, 0x4ef8, { 0x8e, 0x3c, 0x0e, 0x33, 0x9f, 0x3b, 0xc2, 0x2b }}

typedef struct _BLUEFIELD_EEPROM_PROTOCOL BLUEFIELD_EEPROM_PROTOCOL;

#define EEPROM_READ   0x1
#define EEPROM_WRITE  0x0
typedef
EFI_STATUS
(EFIAPI *EFI_EEPROM_TRANSFER) (
  IN CONST BLUEFIELD_EEPROM_PROTOCOL *This,
  IN UINT16                          Address,
  IN UINT32                          Length,
  IN UINT8                           *Buffer,
  IN UINT8                           Operation
  );

struct _BLUEFIELD_EEPROM_PROTOCOL {
  EFI_EEPROM_TRANSFER Transfer;
  UINT32              Identifier;
};

extern EFI_GUID gBluefieldEepromProtocolGuid;
#endif

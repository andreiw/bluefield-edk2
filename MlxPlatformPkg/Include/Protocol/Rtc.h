/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BLUEFIELD_RTC_H__
#define __BLUEFIELD_RTC_H__

#define BLUEFIELD_RTC_PROTOCOL_GUID \
    { 0xd35605e4, 0x5011, 0x42a2, { 0xa9, 0x48, 0x24, 0xdf, 0x48, 0xa4, 0xc9, 0x97 }}

typedef struct _BLUEFIELD_RTC_PROTOCOL BLUEFIELD_RTC_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_RTC_GET_TIME) (
  IN CONST BLUEFIELD_RTC_PROTOCOL *This,
  IN OUT UINT16  *Year,
  IN OUT UINT8   *Month,
  IN OUT UINT8   *Day,
  IN OUT UINT8   *Hour,
  IN OUT UINT8   *Minute,
  IN OUT UINT8   *Second
  );

typedef
EFI_STATUS
(EFIAPI *EFI_RTC_SET_TIME) (
  IN CONST BLUEFIELD_RTC_PROTOCOL *This,
  IN UINT16  Year,
  IN UINT8   Month,
  IN UINT8   Day,
  IN UINT8   Hour,
  IN UINT8   Minute,
  IN UINT8   Second
  );

struct _BLUEFIELD_RTC_PROTOCOL {
  EFI_RTC_GET_TIME   GetTime;
  EFI_RTC_SET_TIME   SetTime;
  UINT32             Identifier;
};

extern EFI_GUID gBluefieldRtcProtocolGuid;
#endif

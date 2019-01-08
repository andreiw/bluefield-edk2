/** @file
  Data structure used by the SysConfig driver.

Copyright (c) 2009 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made
available under the terms and conditions of the BSD License which
accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BLUEFIELD_SYS_CONFIG_H__
#define __BLUEFIELD_SYS_CONFIG_H__

#include <Guid/HiiPlatformSetupFormset.h>
#include <Guid/HiiFormMapMethodGuid.h>
#include <Guid/MlxSysConfigHii.h>
#include <Guid/ZeroGuid.h>

#define SYS_CONFIG_VAR      L"BfSysCfg"

//
// Forms definition
//
#define FORMID_SYS_CONFIG_FORM      1

//
// Key definition
//
#define KEY_SET_PASSWORD              0x2000
#define KEY_GET_PASSWORD              0x2001
#define KEY_RESET_EFI_VAR             0x2002

#define CREDENTIAL_LEN        20

#pragma pack(1)
typedef struct {      // Total 64 bytes
  // Encoded password
  UINT8            Password[CREDENTIAL_LEN];

  // Enable/Disable SMMU
  BOOLEAN          EnableSmmu;

  // Disable SPMI in ACPI
  BOOLEAN          DisableSpmi;

  // Enable the second eMMC card
  BOOLEAN          EnableEmmc1;

  // SPCR UART#.
  UINT8            SPCRPort;

  // Reserved space
  UINT8            Reserved[40];
} SYS_CONFIG;
#pragma pack()

#endif

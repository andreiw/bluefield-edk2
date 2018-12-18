/** @file
  SysConfig driver header file.

Copyright (c) 2018, Mellanox Technologies. All rights reserved.

This program and the accompanying materials are licensed and made available
under the terms and conditions of the BSD License which accompanies this
distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include <Uefi.h>
#include <Guid/GlobalVariable.h>
#include <Guid/MdeModuleHii.h>

#include <Protocol/FormBrowser2.h>
#include <Protocol/FormBrowserEx.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/HiiConfigKeyword.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "BlueFieldSysConfig.h"

extern UINT8      SysConfigStrings[];
extern UINT8      SysConfigVfrBin[];

///
/// HII specific Vendor Device Path definition.
///
typedef struct {
  VENDOR_DEVICE_PATH        VendorDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  End;
} HII_VENDOR_DEVICE_PATH;

#define SYS_CONFIG_SIGNATURE  SIGNATURE_32 ('S', 'C', 'F', 'G')

#define MLX_SYS_CONFIG_HII_VENDOR_DEVICE_PATH(path) \
HII_VENDOR_DEVICE_PATH path = { \
  { \
    { \
      HARDWARE_DEVICE_PATH, \
      HW_VENDOR_DP, \
      { \
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)), \
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8) \
      } \
    }, \
    MLX_SYS_CONFIG_GUID \
  }, \
  { \
    END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, \
    { \
      (UINT8) (END_DEVICE_PATH_LENGTH), \
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8) \
    } \
  } \
}

typedef struct SYS_CONFIG_PRIVATE_DATA {
  UINTN                           Signature;
  EFI_HANDLE                      DriverHandle;
  EFI_HII_HANDLE                  HiiHandle;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;

  SYS_CONFIG                      Configuration;
  UINT8                           PasswordState;

  //
  // Produced protocol.
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL  ConfigAccess;

  //
  // Authentication method.
  //
  VOID (*AuthCheck)(EFI_HII_CONFIG_ACCESS_PROTOCOL *This);
} SYS_CONFIG_PRIVATE_DATA;

#define SYS_CONFIG_PRIVATE_FROM_THIS(a)  CR (a, SYS_CONFIG_PRIVATE_DATA, ConfigAccess, SYS_CONFIG_SIGNATURE)

#endif

/** @file
  Head file for BDS Platform specific code

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _BDS_PLATFORM_H_
#define _BDS_PLATFORM_H_

#include <Protocol/DevicePathFromText.h>
#include <Protocol/DevicePathToText.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/GenericBdsLib.h>
#include <Library/PlatformBdsLib.h>

#include <Guid/GlobalVariable.h>

#include <BootParamFileSystem.h>

/**
  This function runs some unit tests.

**/
VOID
RunTests (
  );

/**
  Create a boot option based on the PcdDefault and the boot parameter
  file system.

  @param BootOptionList          The header of the boot option linked list, to
                                 which the new boot option will be attached.

**/
EFI_STATUS
EFIAPI
CreateDefaultBootOption (
  IN LIST_ENTRY                      *BootOptionList
  );

/**
  Authention user if password has been enabled.

**/
VOID
EFIAPI
PlatformBdsAuth (
  VOID
  );

#endif // _BDS_PLATFORM_H

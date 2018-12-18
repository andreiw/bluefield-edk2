/** @file

  Copyright (c) 2014-2015, ARM Ltd. All rights reserved.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BLUEFIELD_INTERNAL_H__
#define __BLUEFIELD_INTERNAL_H__

#include <BootParamFileSystem.h>
#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "BlueFieldEfiInfo.h"
#include "BlueFieldSysConfig.h"

typedef struct {
  // Flattened Device Tree (FDT) File
  CONST CHAR16              *Name; /// Name of the FDT when present into a File System
  CONST EFI_GUID            *Guid; /// Name of the FDT when present into the FV
} MLNX_PLATFORM;

// Array that contains the list of the Mellanox platforms supported by this DXE driver
extern CONST MLNX_PLATFORM MlnxPlatforms[];

// MLNX system configuration
extern SYS_CONFIG MlnxSysConfig;

/**

  Get information about the Mellanox platform the firmware is running on.

  @param[out]  Platform   Address where the pointer to the platform information
                          (type MLNX_PLATFORM*) should be stored.
                          The returned pointer does not point to an allocated
                          memory area.

  @retval  EFI_SUCCESS    The platform information was returned.
  @retval  EFI_NOT_FOUND  The platform was not recognised.

**/
EFI_STATUS
MlnxGetPlatform (
  OUT CONST MLNX_PLATFORM** Platform
  );

// Get Tile Info
UINT32
MlnxGetTileInfo (
  UINT16 SnicModel
  );

// Install ACPI NVDIMM tables.
EFI_STATUS
BlueFieldAcpiInstallNvdimm (
  void
  );

EFI_STATUS
BlueFieldAcpiInstallMadt (
  UINTN Cores,
  UINT64 CoreEnable
  );

EFI_STATUS
MlnxGetAcpi (
  OUT CONST MLNX_PLATFORM** Platform
  );

BOOLEAN
BlueFieldAcpiCheck (
  IN EFI_ACPI_DESCRIPTION_HEADER *AcpiHeader
  );

#endif // __BLUEFIELD_INTERNAL_H__

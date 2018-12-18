/** @file
*
*  Copyright (c) 2011-2013, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/IoLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

#include <Ppi/ArmMpCoreInfo.h>

#include <ArmPlatform.h>

ARM_CORE_INFO mBlueFieldMpCoreInfoTable[] = {
  {
    // Cluster 0, Core 0
    0x0, 0x0,
  },
  {
    // Cluster 0, Core 1
    0x0, 0x1,
  },
  {
    // Cluster 1, Core 0
    0x1, 0x0,
  },
  {
    // Cluster 1, Core 1
    0x1, 0x1,
  },
  {
    // Cluster 1, Core 0
    0x2, 0x0,
  },
  {
    // Cluster 2, Core 1
    0x2, 0x1,
  },
  {
    // Cluster 3, Core 0
    0x3, 0x0,
  },
  {
    // Cluster 3, Core 1
    0x3, 0x1,
  },
  {
    // Cluster 4, Core 0
    0x4, 0x0,
  },
  {
    // Cluster 4, Core 1
    0x4, 0x1,
  },
  {
    // Cluster 5, Core 0
    0x5, 0x0,
  },
  {
    // Cluster 5, Core 1
    0x5, 0x1,
  },
  {
    // Cluster 6, Core 0
    0x6, 0x0,
  },
  {
    // Cluster 6, Core 1
    0x6, 0x1,
  },
  {
    // Cluster 7, Core 0
    0x7, 0x0,
  },
  {
    // Cluster 7, Core 1
    0x7, 0x1,
  }
};

/**
  Return the current Boot Mode

  This function returns the boot reason on the platform

  @return   Return the current Boot Mode of the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  Initialize controllers that must setup in the normal world

  This function is called by the ArmPlatformPkg/Pei or ArmPlatformPkg/Pei/PlatformPeim
  in the PEI phase.

**/
RETURN_STATUS
ArmPlatformInitialize (
  IN  UINTN                     MpId
  )
{
  return RETURN_SUCCESS;
}

/**
  Initialize the system (or sometimes called permanent) memory

  This memory is generally represented by the DRAM.

**/
VOID
ArmPlatformInitializeSystemMemory (
  VOID
  )
{
  // Nothing to do here
}

EFI_STATUS
PrePeiCoreGetMpCoreInfo (
  OUT UINTN                   *CoreCount,
  OUT ARM_CORE_INFO           **ArmCoreTable
  )
{
  *CoreCount    = ArmGetCpuCountPerCluster ();
  *ArmCoreTable = mBlueFieldMpCoreInfoTable;
  return EFI_SUCCESS;
}

// Needs to be declared in the file. Otherwise gArmMpCoreInfoPpiGuid is undefined in the contect of PrePeiCore
EFI_GUID mArmMpCoreInfoPpiGuid = ARM_MP_CORE_INFO_PPI_GUID;
ARM_MP_CORE_INFO_PPI mMpCoreInfoPpi = { PrePeiCoreGetMpCoreInfo };

EFI_PEI_PPI_DESCRIPTOR      gPlatformPpiTable[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &mArmMpCoreInfoPpiGuid,
    &mMpCoreInfoPpi
  }
};

VOID
ArmPlatformGetPlatformPpiList (
  OUT UINTN                   *PpiListSize,
  OUT EFI_PEI_PPI_DESCRIPTOR  **PpiList
  )
{
  *PpiListSize = sizeof(gPlatformPpiTable);
  *PpiList = gPlatformPpiTable;
}

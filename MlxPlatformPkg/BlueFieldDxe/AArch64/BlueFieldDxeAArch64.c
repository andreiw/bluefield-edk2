/** @file

  Copyright (c) 2014-2015, ARM Ltd. All rights reserved.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/IoLib.h>
#include <Simulator.h>
#include <IndustryStandard/Acpi.h>

#include "rsh_def.h"
#include "BlueFieldInternal.h"
#include "BlueFieldPlatform.h"
#include "BlueFieldEfiInfo.h"

#define TYU_LIVEFISH_MODE_ADDR		0x0
#define TYU_LIVEFISH_MODE_RMASK		0x1

//
// The complete list of supported Mlnx platforms.
//
CONST MLNX_PLATFORM MlnxPlatforms[] = {
  { L"bf2.dtb", FixedPcdGetPtr (PcdFdtBf2) },
  { L"bf4.dtb", FixedPcdGetPtr (PcdFdtBf4) },
  { L"bf16.dtb" , FixedPcdGetPtr (PcdFdtBf16) },
  { L"bf-mini.dtb", FixedPcdGetPtr (PcdFdtBfMini) },
  { L"bf-full.dtb", FixedPcdGetPtr (PcdFdtBfFull) },
  { NULL }
};

STATIC CONST EFI_GUID mBfAcpiTableFile = { 0x6694dfea, 0x6a77, 0x45f8,
    { 0xb8, 0xd1, 0x54, 0xbd, 0x45, 0xe7, 0x21, 0x81 } };
STATIC CONST EFI_GUID mNoTrioTableFile = { 0x2ff16b22, 0xb9b2, 0x4630,
    { 0xaa, 0x7f, 0xcd, 0xd1, 0xb4, 0x74, 0x34, 0x24 } };

CONST MLNX_PLATFORM MlnxAcpiPlatforms[] = {
  { L"bf-full", &mBfAcpiTableFile },
  { L"bf-no-pci", &mNoTrioTableFile },
  { NULL }
};

/**
  Get information about the Mlnx platform the firmware is running on.

  @param[out]  Platform   Address where the pointer to the platform information
                          (type MLNX_PLATFORM*) should be stored.  The returned
                          pointer does not point to an allocated memory area.

  @retval  EFI_SUCCESS    The platform information was returned.
  @retval  EFI_NOT_FOUND  The platform was not recognised.

**/
EFI_STATUS
MlnxGetPlatform (
  OUT CONST MLNX_PLATFORM** Platform
  )
{
  UINTN    Index;
  CHAR16  *BootDtb;
  UINT64 RshFabDim, CoreEnable, TileEnable;
  UINTN  Tiles, Cores, CoresPerTile;
  EFI_STATUS    Status;
  MLNX_EFI_INFO *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;

  ASSERT (Platform != NULL);

  RshFabDim = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_FABRIC_DIM);
  Tiles = ((RshFabDim >> RSH_FABRIC_DIM__DIM_Y_SHIFT) &
           RSH_FABRIC_DIM__DIM_Y_RMASK) *
          ((RshFabDim >> RSH_FABRIC_DIM__DIM_X_SHIFT) &
           RSH_FABRIC_DIM__DIM_X_RMASK);
  CoresPerTile = (((RshFabDim >> RSH_FABRIC_DIM__DIM_Z_SHIFT) &
                   RSH_FABRIC_DIM__DIM_Z_RMASK) + 1);
  Cores = CoresPerTile * Tiles;
  TileEnable = MlnxGetTileInfo(Info->SnicModel);

  if (IsSimulator ()) {
    switch (Cores) {
    case 2:
      BootDtb = L"bf2.dtb";
      break;
    case 4:
      BootDtb = L"bf4.dtb";
      break;
    case 16:
      BootDtb = L"bf16.dtb";
      break;
    default:
      DEBUG((EFI_D_ERROR, "Unsupported configuration: %d tiles %d cores %lx\n",
             Tiles, Cores, RshFabDim));
      return (EFI_NOT_FOUND);
    }
  }
  else {
    if (Tiles >= 8) {
      if (Tiles != 8)
        DEBUG((EFI_D_ERROR, "Not able to enable >8 tiles!\n"));
      BootDtb = L"bf-full.dtb";
    }
    else if (Tiles >= 2) {
      if (Tiles != 2)
        DEBUG((EFI_D_ERROR, "Not able to enable tiles >2 but <8!\n"));
      BootDtb = L"bf-mini.dtb";
    }
    else {
      DEBUG((EFI_D_ERROR, "Can't boot single-tile model!\n"));
      return EFI_NOT_FOUND;
    }
  }

  DEBUG((EFI_D_ERROR, "Number of Tiles:%d Cores:%d Dtb:%s\n",
         Tiles, Cores, BootDtb));

  Status = EFI_NOT_FOUND;
  for (Index = 0; MlnxPlatforms[Index].Name != NULL; Index++) {
    if (StrCmp (MlnxPlatforms[Index].Name, BootDtb) == 0) {
      *Platform = &MlnxPlatforms[Index];
      Status = EFI_SUCCESS;
      break;
    }
  }

  CoreEnable = 0;
  for (Index = 0; Index < Cores; Index++) {
    if (TileEnable & (1 << (Index / CoresPerTile)))
      CoreEnable |= (1 << Index);
  }
  Status = BlueFieldAcpiInstallMadt(Cores, CoreEnable);

  return Status;
}

UINT32
MlnxGetTileInfo (
  UINT16 SnicModel
  )
{
  UINT64 TileEnable, RshTileStatus;

  RshTileStatus = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_TILE_STATUS);
  TileEnable = (RshTileStatus >> RSH_TILE_STATUS__CLUSTER_ENA_SHIFT) &
    RSH_TILE_STATUS__CLUSTER_ENA_RMASK;

  if (SnicModel == 3)
    TileEnable &= 0x1;
  else if (SnicModel == 2)
    TileEnable &= 0x3;
  else if (SnicModel == 1)
    TileEnable &= 0xf;
  else
    TileEnable &= 0xff;

  return TileEnable;
}

EFI_STATUS
MlnxGetAcpi (
  OUT CONST MLNX_PLATFORM** Platform
  )
{
  UINTN Index;
  CHAR16 *TableName;
  EFI_STATUS Status;

  TableName = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootAcpi);
  if (StrCmp (TableName, L"default") == 0) {
    if (MmioRead32(BLUEFIELD_TYU_BASE + TYU_LIVEFISH_MODE_ADDR)
             & TYU_LIVEFISH_MODE_RMASK) {
      TableName = L"bf-no-pci";
      DEBUG ((EFI_D_INFO, "In Livefish mode\n"));
    }
    else
      TableName = L"bf-full";
  }

  DEBUG ((EFI_D_ERROR, "MlnxGetAcpi: TableName:%s\n", TableName));
  Status = EFI_NOT_FOUND;
  for (Index = 0; MlnxAcpiPlatforms[Index].Name != NULL; Index++) {
    if (StrCmp (MlnxAcpiPlatforms[Index].Name, TableName) == 0) {
      *Platform = &MlnxAcpiPlatforms[Index];
      Status = EFI_SUCCESS;
      break;
    }
  }
  return Status;
}

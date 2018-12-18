/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/ArmLib.h>
#include <Simulator.h>

#include "BlueFieldInternal.h"
#include "BlueFieldPlatform.h"
#include "../AcpiTables/Iort.h"
#include "rsh_def.h"
#include "../../../AcpiTables/AcpiTables/OUTPUT/Dsdt.offset.h"

// Simple string search and value update.
// This function doesn't do NULL-terminated check. The caller should
// make sure that the searched string exists and the data type is 'byte'.
// Argument:
//      Addr      The start address to do the string search
//      Len       The length from the start address of the search
//      Str       ASCII string to search
//      Value     Byte value to be replaced with
//      Checksum  Checksum used for incremental update
static VOID
PatchStringByteValue(
  UINT8         *Addr,
  UINT32        Len,
  CHAR8         *Str,
  UINT8         Value,
  UINT8         *Checksum
  )
{
  CHAR8        *Next = (CHAR8 *)Addr, *Find;

  while ((Find = AsciiStrStr(Next, Str)) == NULL) {
    Next += AsciiStrLen(Next);
    while (*Next == 0) {
      Next++;
      // Return if out of range. In such case, patch doesn't happen.
      if ((UINT8 *)Next - Addr >= Len)
        return;
    }
    if ((UINT8 *)Next - Addr >= Len)
      break;
  }

  // Format: <Str> 0x00 0x0a <Value>
  if (Find != NULL) {
    Find += AsciiStrLen(Find);
    // Incremental checksum update
    *Checksum -= (Value - Find[2]);
    Find[2] = Value;
  }
}

static VOID
PatchDsdtTable (
  IN EFI_ACPI_DESCRIPTION_HEADER *AcpiHeader
  )
{
  int                    i;
  AML_OFFSET_TABLE_ENTRY *DsdtOffsetTable = &DSDT_MLX_BF01_OffsetTable[0];
  MLNX_EFI_INFO          *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;
  UINT8                  *Hdr;
  UINT32                 Len;
  UINT64                 TileEnable, TileCount;

  for (i = 0; DsdtOffsetTable[i].Pathname != NULL; i++) {
    if (AsciiStrCmp(DsdtOffsetTable[i].Pathname, "_SB_.MMC0._DSD") == 0) {
      // Ignore the eMMC patching if the 2nd card is enabled.
      if (Info->SecEmmc && MlnxSysConfig.EnableEmmc1)
        continue;

      // ACPI 6.2 20.2.5.4 Type 2 Opcodes Encoding
      // DefPackage := PackageOp PkgLength NumElements PackageElementList
      // In this case, PkgLength is built from byte 1 & 2
      Hdr = (UINT8 *)AcpiHeader + DsdtOffsetTable[i].Offset;
      Len = ((UINT32)Hdr[2] << 8) + (Hdr[1] & 0x0f);

      // Adjust num-slots to 1.
      PatchStringByteValue(Hdr, Len, "num-slots", 1, &AcpiHeader->Checksum);
    }

    if (AsciiStrCmp(DsdtOffsetTable[i].Pathname, "_SB_.PMC0._DSD") == 0) {
      TileEnable = MlnxGetTileInfo(Info->SnicModel);
      for (TileCount = 0; TileEnable; TileEnable >>= 1)
        TileCount += TileEnable & 1;

      Hdr = (UINT8 *)AcpiHeader + DsdtOffsetTable[i].Offset;
      Len = ((UINT32)Hdr[2] << 8) + (Hdr[1] & 0x0f);

      // Adjust tile count based on SnicModel
      PatchStringByteValue(Hdr, Len, "tile_num", TileCount, &AcpiHeader->Checksum);
    }
  }
}

//
// Check and patch ACPI tables.
// Return FALSE will skip this table during loading.
//
BOOLEAN
BlueFieldAcpiCheck (
  IN EFI_ACPI_DESCRIPTION_HEADER *AcpiHeader
  )
{
  BOOLEAN Valid = TRUE;

  switch (AcpiHeader->OemTableId) {
  case EFI_ACPI_MLNX_OEM_TABLE_ID_DSDT:
    PatchDsdtTable(AcpiHeader);
    break;

  case EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO:
  case EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_MIN:
    // Check whether the PKA hardware exists; For now, we assume that
    // if the A72 cryptography extensions are enabled then the chip is
    // a High/bin Crypto. This is weak but we should update it in the
    // future (e.g: passed through HOB, based on eFuse, or passed by
    // ATF code). Here we check for the AES instructions; a non-zero
    // value indicates that those instructions are implemented.
    Valid = (ArmGetCpuIsarBit (ARM_CPU_ISAR_AES_MASK) != 0) ?
                    TRUE : (!IsSimulator ()) ? FALSE : TRUE;

    // Now, if The PKA hardware exists and the SMMU support is on,
    // then we can load the full model including the ring devices.
    // On the other hand if the SMMU support is disabled then the
    // ring devices cannot be used and thus load the mini model.
    // The mini model allows the usage of the TRNG engine only.

    if (Valid &&
         AcpiHeader->OemTableId == EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_MIN)
      Valid = (MlnxSysConfig.EnableSmmu) ? FALSE : TRUE;

    if (Valid &&
         AcpiHeader->OemTableId == EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO)
      Valid = (MlnxSysConfig.EnableSmmu) ? TRUE : FALSE;

    break;

  case EFI_ACPI_MLNX_OEM_TABLE_ID_PCI:
    // Return FALSE here to skip the PCI table if needed.
    break;

  case EFI_ACPI_MLNX_OEM_TABLE_ID_SPCR:
    //
    // Possibly patch in the future if required.
    //
    break;

  case EFI_ACPI_MLNX_OEM_TABLE_ID_IORT: {
    EFI_ACPI_6_1_IORT_STRUCTURE *Iort;

    Iort = (EFI_ACPI_6_1_IORT_STRUCTURE *)AcpiHeader;
    if (MlnxSysConfig.EnableSmmu) {
      Iort->RcNode.IdMapping[0].OutputRefernce = OFFSET_OF(EFI_ACPI_6_1_IORT_STRUCTURE, SmmuNode);
      Iort->SmmuNode.Type = EFI_ACPI_6_1_IORT_TYPE_SMMU_V1;
    } else {
      Iort->RcNode.IdMapping[0].OutputRefernce = OFFSET_OF(EFI_ACPI_6_1_IORT_STRUCTURE, ItsNode);
      Iort->SmmuNode.Type = EFI_ACPI_6_1_IORT_TYPE_INVALID;
    }
    break;
    }

  case EFI_ACPI_MLNX_OEM_TABLE_ID_SPMI:
    // Skip the IPMI table if disabled in the UEFI config.
    if (MlnxSysConfig.DisableSpmi)
      return FALSE;
    break;

  default:
    break;
  }

  return Valid;
}

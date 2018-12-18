/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BlueFieldInternal.h"
#include "BlueFieldPlatform.h"

#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/AcpiLib.h>
#include <Protocol/AcpiTable.h>
#include <IndustryStandard/Acpi.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>

// NFIT table header template.
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER           Header;
  UINT32                                Reserved;
} NFIT_TABLE_HDR;
STATIC NFIT_TABLE_HDR NfitTableHdr = {
  .Header = MLNX_ACPI_HEADER(EFI_ACPI_6_1_NVDIMM_FIRMWARE_INTERFACE_TABLE_STRUCTURE_SIGNATURE,
                             EFI_ACPI_DESCRIPTION_HEADER,
                             0x01,
                             EFI_ACPI_MLNX_OEM_TABLE_ID)
};

// NFIT AddressRange template.
STATIC EFI_ACPI_6_1_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_STRUCTURE AddressRangeTemplate = {
    .Type                   = EFI_ACPI_6_1_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_STRUCTURE_TYPE,
    .Length                 = 0x0038,
    .SPARangeStructureIndex = 0x0002,
    .Flags                  = EFI_ACPI_6_1_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_FLAGS_PROXIMITY_DOMAIN_VALID |
                              EFI_ACPI_6_1_NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE_FLAGS_CONTROL_REGION_FOR_MANAGEMENT,
    .ProximityDomain        = 0x00000000,
    .AddressRangeTypeGUID   = (GUID)EFI_ACPI_6_1_NFIT_GUID_BYTE_ADDRESSABLE_PERSISTENT_MEMORY_REGION,
    .SystemPhysicalAddressRangeBase     = 0x0000000180000000UL,
    .SystemPhysicalAddressRangeLength   = 0x0000000001000000UL,
    .AddressRangeMemoryMappingAttribute = EFI_MEMORY_WB | EFI_MEMORY_NV,
};

// NFIT RegionMap template.
STATIC EFI_ACPI_6_1_NFIT_NVDIMM_REGION_MAPPING_STRUCTURE RegionMapTemplate = {
    .Type                   = EFI_ACPI_6_1_NFIT_NVDIMM_REGION_MAPPING_STRUCTURE_TYPE,
    .Length                 = 0x0030,
    .NFITDeviceHandle       = {
                                .DIMMNumber = 1,
                                .MemoryChannelNumber = 0,
                                .MemoryControllerID = 0,
                                .SocketID = 0,
                                .NodeControllerID = 0
                              },
    .NVDIMMPhysicalID       = 0x0000,
    .NVDIMMRegionID         = 0x0000,
    .SPARangeStructureIndex = 0x0002,
    .NVDIMMControlRegionStructureIndex  = 0x0003,
    .NVDIMMRegionSize       = 0x0000000001000000UL,
    .RegionOffset           = 0x0000000000000000UL,
    .NVDIMMPhysicalAddressRegionBase    = 0x0000000000000000UL,
    .InterleaveStructureIndex   = 0x0000,
    .InterleaveWays         = 0x0001,
    .NVDIMMStateFlags       = 0x0000,
};

// NFIT ControlRegion template.
STATIC EFI_ACPI_6_1_NFIT_NVDIMM_CONTROL_REGION_STRUCTURE ControlRegionTemplate = {
    .Type                   = EFI_ACPI_6_1_NFIT_NVDIMM_CONTROL_REGION_STRUCTURE_TYPE,
    .Length                 = 0x0050,
    .NVDIMMControlRegionStructureIndex  = 0x0003,
    .VendorID               = 0x8086,
    .DeviceID               = 0x0001,
    .RevisionID             = 0x0001,
    .SubsystemVendorID      = 0x0000,
    .SubsystemDeviceID      = 0x0000,
    .SubsystemRevisionID    = 0x0000,
    .SerialNumber           = 0x00123456,
    .RegionFormatInterfaceCode      = 0x0201,
    .NumberOfBlockControlWindows    = 0x0000,
    .SizeOfBlockControlWindow       = 0x0000000000000000UL,
    .CommandRegisterOffsetInBlockControlWindow  = 0x0000000000000000UL,
    .SizeOfCommandRegisterInBlockControlWindows = 0x0000000000000000UL,
    .StatusRegisterOffsetInBlockControlWindow   = 0x0000000000000000UL,
    .SizeOfStatusRegisterInBlockControlWindows  = 0x0000000000000000UL,
    .NVDIMMControlRegionFlag        = 0x0000,
};

EFI_STATUS
BlueFieldAcpiInstallNvdimm (
  void
  )
{
  int                           i;
  EFI_STATUS                    Status;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol;
  UINTN                         AcpiTableKey;
  EFI_GUID                      Guid = EFI_ACPI_TABLE_PROTOCOL_GUID;
  UINT8                         *NfitTable, *DataPtr;
  UINT32                        Size;
  EFI_ACPI_DESCRIPTION_HEADER   *Hdr;
  MLNX_EFI_INFO                 *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;
  MLNX_EFI_INFO_MEM_REGION      *Region;
  MLNX_EFI_INFO_NVDIMM          *Nvdimm;

  if (Info->RegionNum == 0 || Info->NvdimmNum == 0)
    return EFI_INVALID_PARAMETER;

  if (Info->NvdimmNum > NVDIMM_MAX_NUM)
    Info->NvdimmNum = NVDIMM_MAX_NUM;

  // Ensure the ACPI Table is present
  Status = gBS->LocateProtocol (
                  &Guid,
                  NULL,
                  (VOID**)&AcpiProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Size = sizeof(NfitTableHdr) + NVDIMM_MAX_NUM * (sizeof(AddressRangeTemplate) +
                sizeof(RegionMapTemplate) + sizeof(ControlRegionTemplate));
  NfitTable = AllocatePool(Size);
  ASSERT (NfitTable != NULL);

  DataPtr = NfitTable;
  for (i = 0; i < Info->NvdimmNum; i++) {
    Nvdimm = &Info->Nvdimm[i];
    Region = &Info->Region[Nvdimm->DimmId];

    if (NfitTable == DataPtr) {
      CopyMem(DataPtr, &NfitTableHdr, sizeof(NfitTableHdr));
      DataPtr += sizeof(NfitTableHdr);
    }

    AddressRangeTemplate.SPARangeStructureIndex = i + 1;
    AddressRangeTemplate.SystemPhysicalAddressRangeBase = Region->PhyAddr;
    AddressRangeTemplate.SystemPhysicalAddressRangeLength = Region->Length;
    CopyMem(DataPtr, &AddressRangeTemplate, sizeof(AddressRangeTemplate));
    DataPtr += sizeof(AddressRangeTemplate);

    RegionMapTemplate.NVDIMMPhysicalID = i;
    RegionMapTemplate.NVDIMMRegionID = i;
    RegionMapTemplate.SPARangeStructureIndex = i + 1;
    RegionMapTemplate.NVDIMMControlRegionStructureIndex = (i + 1) << 8;
    RegionMapTemplate.NVDIMMRegionSize = Region->Length;
    RegionMapTemplate.NFITDeviceHandle.DIMMNumber = Nvdimm->DimmId + 1;
    RegionMapTemplate.NFITDeviceHandle.MemoryChannelNumber = Region->MemoryChannelNumber;
    RegionMapTemplate.NFITDeviceHandle.MemoryControllerID = Region->MemoryControllerID;
    RegionMapTemplate.NFITDeviceHandle.SocketID = Region->SocketID;
    CopyMem(DataPtr, &RegionMapTemplate, sizeof(RegionMapTemplate));
    DataPtr += sizeof(RegionMapTemplate);

    ControlRegionTemplate.NVDIMMControlRegionStructureIndex = (i + 1) << 8;
    ControlRegionTemplate.VendorID = *(UINT16 *)Nvdimm->VendorID;
    ControlRegionTemplate.DeviceID = *(UINT16 *)Nvdimm->DeviceID;
    ControlRegionTemplate.RevisionID = *(UINT16 *)Nvdimm->RevisionID;
    ControlRegionTemplate.SubsystemVendorID = *(UINT16 *)Nvdimm->SubsystemVendorID;
    ControlRegionTemplate.SubsystemDeviceID = *(UINT16 *)Nvdimm->SubsystemDeviceID;
    ControlRegionTemplate.SubsystemRevisionID = *(UINT16 *)Nvdimm->SubsystemRevisionID;
    ControlRegionTemplate.ManufacturingLocation = Nvdimm->ManufacturingLocation[0];
    ControlRegionTemplate.ManufacturingDate = *(UINT16 *)Nvdimm->ManufacturingData;
    ControlRegionTemplate.SerialNumber = *(UINT32 *)Nvdimm->SerialNumber;
    ControlRegionTemplate.RegionFormatInterfaceCode = *(UINT16*)Nvdimm->RegionFormatInterfaceCode;
    CopyMem(DataPtr, &ControlRegionTemplate, sizeof(ControlRegionTemplate));
    DataPtr += sizeof(ControlRegionTemplate);

    // Update the table size.
    Hdr = (EFI_ACPI_DESCRIPTION_HEADER *)NfitTable;
    Hdr->Length = DataPtr - NfitTable;
  }

  if (DataPtr == NfitTable)
    return EFI_NOT_FOUND;

  Status = AcpiProtocol->InstallAcpiTable (
                       AcpiProtocol,
                       (EFI_ACPI_COMMON_HEADER *)NfitTable,
                       DataPtr - NfitTable,
                       &AcpiTableKey
                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FreePool(NfitTable);

  return 0;
}

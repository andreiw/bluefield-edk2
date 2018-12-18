/** @file

  Copyright (c) 2014-2015, ARM Ltd. All rights reserved.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License which accompanies this
  distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __BLUEFIELD_EFIINFO_H__
#define __BLUEFIELD_EFIINFO_H__

// 4K bytes
#define MLNX_EFI_INFO_SIZE      (4 * 1024)

// We have max 4 dimm slots.
#define DIMM_MAX_NUM        4

// Support up to 3 NVDIMMs.
#define NVDIMM_MAX_NUM      (DIMM_MAX_NUM - 1)

// Structure for type MLNX_EFI_INFO_TYPE_MEM_REGION
typedef struct {
  UINT32                    SocketID : 4;
  UINT32                    MemoryControllerID : 4;
  UINT32                    MemoryChannelNumber : 4;
  UINT32                    IsNvdimm : 1;   /// Memory region created for NVDIMM
  UINT32                    DimmId : 4;     /// DIMM ID starting from 0
  UINT64                    PhyAddr;        /// Physical address
  UINT64                    Length;         /// Length of the memory region
} MLNX_EFI_INFO_MEM_REGION;

// Structure for type MLNX_EFI_INFO_TYPE_NVDIMM
typedef struct {
  // Data from SPD.
  UINT8                     VendorID[2];
  UINT8                     DeviceID[2];
  UINT8                     RevisionID[2];
  UINT8                     SubsystemVendorID[2];
  UINT8                     SubsystemDeviceID[2];
  UINT8                     SubsystemRevisionID[2];
  UINT8                     ManufacturingLocation[1];
  UINT8                     ManufacturingData[2];
  UINT8                     SerialNumber[4];
  UINT8                     RegionFormatInterfaceCode[2];

  // DIMM ID of type MLNX_EFI_INFO_TYPE_MEM_REGION
  UINT8                     DimmId;
} MLNX_EFI_INFO_NVDIMM;

//
// NVDIMM ARS (Address Range Scrub) structure
//
#define NVDIMM_ARS_OFF          0x800
#define NVDIMM_ARS_SIZE         0x400

typedef struct {
  UINT8 data[NVDIMM_ARS_SIZE];
} __attribute__((aligned(NVDIMM_ARS_OFF))) MLNX_ARS_INFO;

// Structure used to pass information from ATF.
// It contains sub structs to describe the memory we have as well as any
// particular NVDIMM info that we need.
typedef struct {
  UINT32                    Reserved;   /// Reserved for a jump to UEFI code.
  UINT8                     Magic[4];   /// Magic Number

  //
  // DIMM / NVDIMM information
  //
  UINT8                     RegionNum;  /// Number of valid INFO_MEM_REGION
  UINT8                     NvdimmNum;  /// Number of valid INFO_NVDIMM

  UINT16                    SecEmmc : 1; /// Enable second eMMC
  UINT16                    ExtBoot : 1; /// External boot
  UINT16                    TmFifoInit : 1; /// TmFifo init flag
  UINT16                    BfIsPal : 1; /// Running on Palladium
  UINT16                    SnicModel : 2; /// Model of SmartNic

  // Struct array to store the memory region information.
  // The index to this array is calculated as:
  // index = mss_num * MAX_DIMM_PER_MEM_CTRL + dimm_slot_num
  // So there may be cases where a low index doesn't have a valid
  // entry but a higher index does.
  MLNX_EFI_INFO_MEM_REGION  Region[DIMM_MAX_NUM];
  // Struct array to store the NVDIMM specific information.
  // This array is always filled up from index 0 to the number of
  // NVDIMMs we have.
  MLNX_EFI_INFO_NVDIMM      Nvdimm[NVDIMM_MAX_NUM];

  // Save the TmFifo pointer to be shared among UEFI components.
  VOID                     *TmFifo;

  // Pointer of the EFI System Table.
  VOID                     *EfiSysTbl;

  //
  // NVDIMM ARS (Address Range Scrub) structure
  //
  MLNX_ARS_INFO             Ars;
} MLNX_EFI_INFO;

#define MLNX_EFI_INFO_ADDR   ((MLNX_EFI_INFO *)TILE_UEFI_IMAGE_ADDR)

#endif // __BLUEFIELD_EFIINFO_H__

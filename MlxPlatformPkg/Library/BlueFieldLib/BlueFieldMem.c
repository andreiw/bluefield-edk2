/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*  Copyright (c) 2015, Mellanox Semiconductor, Ltd.  All rights reserved.
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

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Simulator.h>
#include <BlueFieldPlatform.h>
#include <BlueFieldEfiInfo.h>

#include "rsh_def.h"

// Number of Virtual Memory Map Descriptors
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS          12

// DDR attributes
#define DDR_ATTRIBUTES_CACHED           ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED         ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  int                           i;
  ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  EFI_RESOURCE_ATTRIBUTE_TYPE   ResourceAttributes;
  UINTN                         Index = 0;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;
  BOOLEAN                       HasSparseMemory = FALSE;
  EFI_VIRTUAL_ADDRESS           SparseMemoryBase = 0;
  UINT64                        SparseMemorySize = 0;
  UINT64                        MemSizeInGb = 0;
  MLNX_EFI_INFO                 *Info = (MLNX_EFI_INFO *)MLNX_EFI_INFO_ADDR;
  MLNX_EFI_INFO_MEM_REGION      *Region;

  ASSERT (VirtualMemoryMap != NULL);

  // Figure out how much memory we have by adding up the size of all non NVDIMM
  // memory installed.
  ASSERT(Info->RegionNum > 0);
  for (i = 0; i < DIMM_MAX_NUM; i++) {
    Region = &Info->Region[i];
    if (Region->Length > 0 && !(Region->IsNvdimm)) {
      MemSizeInGb += ((Region->Length) >> 30);
    }
  }

  if (MemSizeInGb > 2) {
    // Declare any additional DRAM after the 4GB boundary
    HasSparseMemory = TRUE;
    SparseMemoryBase = 0x0100000000;
    SparseMemorySize = (MemSizeInGb << 30) - SIZE_2GB;
  }

  ResourceAttributes =
      EFI_RESOURCE_ATTRIBUTE_PRESENT |
      EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
      EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
      EFI_RESOURCE_ATTRIBUTE_TESTED;

  BuildResourceDescriptorHob (
      EFI_RESOURCE_SYSTEM_MEMORY,
      ResourceAttributes,
      SparseMemoryBase,
      SparseMemorySize);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages(EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS));
  if (VirtualMemoryTable == NULL) {
      return;
  }

  if (FeaturePcdGet(PcdCacheEnable) == TRUE) {
      CacheAttributes = DDR_ATTRIBUTES_CACHED;
  } else {
      CacheAttributes = DDR_ATTRIBUTES_UNCACHED;
  }

  // UARTs and other rshim peripherals.
  VirtualMemoryTable[Index].PhysicalBase = BLUEFIELD_RSHIM_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_RSHIM_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_RSHIM_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // TYU including CAUSE and SMBUS
  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_TYU_BASE;
  VirtualMemoryTable[Index].VirtualBase   = BLUEFIELD_TYU_BASE;
  VirtualMemoryTable[Index].Length        = BLUEFIELD_TYU_SZ;
  VirtualMemoryTable[Index].Attributes    = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // MMC
  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_MMC_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_MMC_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_MMC_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Simulator I/O, including virtio.
  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_SIM_IO_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_SIM_IO_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_SIM_IO_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // GIC.
  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_GIC_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_GIC_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_GIC_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // DDR
  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_DRAM_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_DRAM_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_DRAM_SZ;
  VirtualMemoryTable[Index].Attributes   = CacheAttributes;

  // PCI
  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_PCI_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_PCI_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_PCI_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_PCI_MEM32_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_PCI_MEM32_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_PCI_MEM32_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  VirtualMemoryTable[++Index].PhysicalBase = BLUEFIELD_PCI_MEM64_BASE;
  VirtualMemoryTable[Index].VirtualBase  = BLUEFIELD_PCI_MEM64_BASE;
  VirtualMemoryTable[Index].Length       = BLUEFIELD_PCI_MEM64_SZ;
  VirtualMemoryTable[Index].Attributes   = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // Map sparse memory region if present
  if (HasSparseMemory) {
    VirtualMemoryTable[++Index].PhysicalBase = SparseMemoryBase;
    VirtualMemoryTable[Index].VirtualBase    = SparseMemoryBase;
    VirtualMemoryTable[Index].Length         = SparseMemorySize;
    VirtualMemoryTable[Index].Attributes     = CacheAttributes;
  }

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase = 0;
  VirtualMemoryTable[Index].VirtualBase  = 0;
  VirtualMemoryTable[Index].Length       = 0;
  VirtualMemoryTable[Index].Attributes   = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT (Index < MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}

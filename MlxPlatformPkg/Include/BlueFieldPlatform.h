/** @file
*  Header defining BlueField constants (Base addresses, sizes, flags)
*
*  Copyright (c) 2015, Mellanox Semiconductor, Ltd.  All rights reserved.
*
*  This program and the accompanying materials are licensed and made
*  available under the terms and conditions of the BSD License which
*  accompanies this distribution.  The full text of the license may be
*  found at http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR
*  IMPLIED.
*
**/

#ifndef __BLUEFIELD_PLATFORM_H__
#define __BLUEFIELD_PLATFORM_H__

/******************************************************************************
// Platform Memory Map
******************************************************************************/

// UARTs and other rshim peripherals.
#define BLUEFIELD_RSHIM_BASE                      0x00800000
#define BLUEFIELD_RSHIM_SZ                        (SIZE_16MB + SIZE_8MB)
#define BLUEFIELD_UART0_BASE                      0x01000000
#define BLUEFIELD_UART1_BASE                      0x01800000

// TYU address space - 0x0002800000..0x00028fffff
// Map only the first 32KB which includes CAUSE, GPIO, PLL and SMBUS.
#define BLUEFIELD_TYU_BASE                        0x02800000
#define BLUEFIELD_TYU_SZ                          SIZE_32KB

// MMC
#define BLUEFIELD_MMC_BASE                        0x06000000
#define BLUEFIELD_MMC_SZ                          SIZE_8MB

// Other I/O, notably Virtio, present only in the simulator environment.
#define BLUEFIELD_SIM_IO_BASE                     0x07030000
#define BLUEFIELD_SIM_IO_SZ                       SIZE_64KB

// GIC.
#define BLUEFIELD_GIC_BASE                        0x11000000
#define BLUEFIELD_GIC_SZ                          SIZE_32MB

// DRAM.
#define BLUEFIELD_DRAM_BASE                       PcdGet64 (PcdSystemMemoryBase)
#define BLUEFIELD_DRAM_SZ                         PcdGet64 (PcdSystemMemorySize)

// PCI.
#define BLUEFIELD_PCI_BASE                        0xe100000000
#define BLUEFIELD_PCI_SZ                          0x10000000
#define BLUEFIELD_PCI_MEM32_BASE                  0xe000000000
#define BLUEFIELD_PCI_MEM32_SZ                    0x100000000
#define BLUEFIELD_PCI_MEM64_BASE                  0xe200000000
#define BLUEFIELD_PCI_MEM64_SZ                    0x1e00000000

// UEFI Image Address
#define TILE_UEFI_IMAGE_ADDR                   (BLUEFIELD_DRAM_BASE + 0x8000000)

/******************************************************************************
// Memory-mapped peripherals
******************************************************************************/

// None so far.

//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_MLNX_OEM_ID           'M','L','N','X','T','.'   // OEMID 6 bytes long
#define EFI_ACPI_MLNX_OEM_ID_STR       "MLNXT."
#define EFI_ACPI_MLNX_OEM_REVISION     0x20170213
#define EFI_ACPI_MLNX_CREATOR_ID       SIGNATURE_32('M','L','N','X')
#define EFI_ACPI_MLNX_CREATOR_REVISION 0x00000099
#define EFI_ACPI_MLNX_OEM_TABLE_ID              SIGNATURE_64('M','L','X','-','B','F','0','0') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_DSDT         SIGNATURE_64('M','L','X','-','B','F','0','1') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_PCI          SIGNATURE_64('M','L','X','-','B','F','0','2') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_NVDIMM       SIGNATURE_64('M','L','X','-','B','F','0','3') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO       SIGNATURE_64('M','L','X','-','B','F','0','4') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_LVF          SIGNATURE_64('M','L','X','-','B','F','0','5') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_SPMI         SIGNATURE_64('M','L','X','-','B','F','0','6') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_IORT         SIGNATURE_64('M','L','X','-','B','F','0','7') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_MIN   SIGNATURE_64('M','L','X','-','B','F','0','8') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_SPCR         SIGNATURE_64('M','L','X','-','B','F','F','F') // OEM table id (8B)
#define EFI_ACPI_MLNX_OEM_TABLE_ID_DSDT_STR         "MLX-BF01"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_PCI_STR          "MLX-BF02"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_NVDIMM_STR       "MLX-BF03"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_STR       "MLX-BF04"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_LVF_STR          "MLX-BF05"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_SPMI_STR         "MLX-BF06"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_IORT_STR         "MLX-BF07"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_MIN_STR   "MLX-BF08"
#define EFI_ACPI_MLNX_OEM_TABLE_ID_SPCR_STR         "MLX-BFFF"

// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
#define MLNX_ACPI_HEADER(Signature, Type, Revision, OemTableID) { \
    Signature,                      /* UINT32  Signature */       \
    sizeof (Type),                  /* UINT32  Length */          \
    Revision,                       /* UINT8   Revision */        \
    0,                              /* UINT8   Checksum */        \
    { EFI_ACPI_MLNX_OEM_ID },        /* UINT8   OemId[6] */        \
    OemTableID,                      /* UINT64  OemTableId */      \
    EFI_ACPI_MLNX_OEM_REVISION,      /* UINT32  OemRevision */     \
    EFI_ACPI_MLNX_CREATOR_ID,        /* UINT32  CreatorId */       \
    EFI_ACPI_MLNX_CREATOR_REVISION   /* UINT32  CreatorRevision */ \
  }

//
// ARM MP Core IDs
//
#define ARM_CORE_AFF0         0xFF
#define ARM_CORE_AFF1         (0xFF << 8)
#define ARM_CORE_AFF2         (0xFF << 16)
#define ARM_CORE_AFF3         (0xFFULL << 32)

#define ARM_CORE_MASK         ARM_CORE_AFF0
#define ARM_CLUSTER_MASK      ARM_CORE_AFF1
#define GET_CORE_ID(MpId)     ((MpId) & ARM_CORE_MASK)
#define GET_CLUSTER_ID(MpId)  (((MpId) & ARM_CLUSTER_MASK) >> 8)
#define GET_MPID(ClusterId, CoreId)   (((ClusterId) << 8) | (CoreId))

//
// ARM Core Instruction Set Attribute
//
#define ARM_CPU_ISAR_AES_MASK  0x30

#endif

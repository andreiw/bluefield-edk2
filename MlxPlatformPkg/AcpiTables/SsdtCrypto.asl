/** @file
*  Secondary System Description Table (SSDT)
*
*  Full Crypto Configuration.
*
*  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
**/

#include "BlueFieldPlatform.h"
#include "Crypto.h"
#include "bf1_irqs.h"

#define CRYPTO_PKA_RING_DEVICE_ENTRY(pka, ring) \
            Device (CRYPTO_PKA_RING_DEVICE_NAME(ring)) \
            { \
                Name (_ADR, CRYPTO_PKA_RING_DEVICE_ADR(pka, ring)) \
                Name (_HID, CRYPTO_PKA_RING_DEVICE_ACPIHID) \
            }

#define CRYPTO_PKA_DEVICE_ENTRY(id, base, irq) \
    Device (CRYPTO_PKA_DEVICE_NAME(id)) \
    { \
      CRYPTO_PKA_DEVICE_PROPERTY (base, irq) \
      \
      CRYPTO_PKA_RING_DEVICE_ENTRY (id, CRYPTO_PKA_RING_0_ID)  /* RING 0 */ \
      CRYPTO_PKA_RING_DEVICE_ENTRY (id, CRYPTO_PKA_RING_1_ID)  /* RING 1 */ \
      CRYPTO_PKA_RING_DEVICE_ENTRY (id, CRYPTO_PKA_RING_2_ID)  /* RING 2 */ \
      CRYPTO_PKA_RING_DEVICE_ENTRY (id, CRYPTO_PKA_RING_3_ID)  /* RING 3 */ \
    }

// AMLFileName, TableSignature, ComplianceRevision, OEMID, TableID, OEMRevision)
DefinitionBlock ("Ssdt.aml", "SSDT", 1, EFI_ACPI_MLNX_OEM_ID_STR, EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_STR, EFI_ACPI_MLNX_OEM_REVISION)
{
  Scope (\_SB)
  {
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_0_PKA_0_ID, CRYPTO_0_PKA_0_BASE, BF1_CRYPTO0_PKA_MASTER_0_INT) // PKA 0
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_0_PKA_1_ID, CRYPTO_0_PKA_1_BASE, BF1_CRYPTO0_PKA_MASTER_1_INT) // PKA 1
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_1_PKA_0_ID, CRYPTO_1_PKA_0_BASE, BF1_CRYPTO1_PKA_MASTER_0_INT) // PKA 2
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_1_PKA_1_ID, CRYPTO_1_PKA_1_BASE, BF1_CRYPTO1_PKA_MASTER_1_INT) // PKA 3
  }
}

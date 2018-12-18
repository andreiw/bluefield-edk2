/** @file
*  Secondary System Description Table (SSDT)
*
*  Minimal Crypto Configuration.
*
*  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
**/

#include "BlueFieldPlatform.h"
#include "Crypto.h"
#include "bf1_irqs.h"

#define CRYPTO_PKA_DEVICE_ENTRY(id, base, irq) \
    Device (CRYPTO_PKA_DEVICE_NAME(id)) \
    { \
      CRYPTO_PKA_DEVICE_PROPERTY (base, irq) \
    }

// AMLFileName, TableSignature, ComplianceRevision, OEMID, TableID, OEMRevision)
DefinitionBlock ("Ssdt.aml", "SSDT", 1, EFI_ACPI_MLNX_OEM_ID_STR, EFI_ACPI_MLNX_OEM_TABLE_ID_CRYPTO_MIN_STR, EFI_ACPI_MLNX_OEM_REVISION)
{
  Scope (\_SB)
  {
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_0_PKA_0_ID, CRYPTO_0_PKA_0_BASE, BF1_CRYPTO0_PKA_MASTER_0_INT) // PKA 0
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_0_PKA_1_ID, CRYPTO_0_PKA_1_BASE, BF1_CRYPTO0_PKA_MASTER_1_INT) // PKA 1
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_1_PKA_0_ID, CRYPTO_1_PKA_0_BASE, BF1_CRYPTO1_PKA_MASTER_0_INT) // PKA 2
    CRYPTO_PKA_DEVICE_ENTRY (CRYPTO_1_PKA_1_ID, CRYPTO_1_PKA_1_BASE, BF1_CRYPTO1_PKA_MASTER_1_INT) // PKA 3
  }
}

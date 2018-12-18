#ifndef __IORT_H__
#define __IORT_H__

#include <Library/AcpiLib.h>
#include <Library/PcdLib.h>
#include <IndustryStandard/Acpi.h>

#include "Crypto.h"

//
// IORT ACPI definition.
//

#define NUM_ITS 1
#define NUM_PCI_SEG 1

#define EFI_ACPI_6_1_IORT_REVISION 0
#define EFI_ACPI_6_1_IORT_TYPE_INVALID -1
#define EFI_ACPI_6_1_IORT_TYPE_ITS 0
#define EFI_ACPI_6_1_IORT_TYPE_NAMED 1
#define EFI_ACPI_6_1_IORT_TYPE_RC 2
#define EFI_ACPI_6_1_IORT_TYPE_SMMU_V1 3
#define EFI_ACPI_6_1_IORT_TYPE_SMMU_V3 4
#define EFI_ACPI_6_1_IORT_MMU_500 3

#define LEVEL_TRIGGER 0
#define EDGE_TRIGGER 1

#define SMMU_CPTW               2       /* Coherent Page Table Walk */
#define MEMORY_ACCESS_CCA       1       /* Cache Coherent Attribute */
#define MEMORY_ACCESS_CPM       (1ULL << 56)  /* Coherent Path to Memory */

#pragma pack (1)

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT32                      NumberOfNodes;
  UINT32                      NodeArrayOffset;
  UINT32                      Reserved_44;
} EFI_ACPI_6_1_IORT_HEADER;

typedef struct {
  UINT8        Type;
  UINT16       Length;
  UINT8        Revision;
  UINT32       Reserved_4;
  UINT32       NumberOfMappings;
  UINT32       IdArray;
  UINT32       NumberOfIts;
  UINT32       ItsIdArray[NUM_ITS];
} EFI_ACPI_6_1_IORT_ITS_NODE;

typedef struct {
  UINT32       InputBase;
  UINT32       NumberOfIds;
  UINT32       OutputBase;
  UINT32       OutputRefernce;
  UINT32       Flags;
} EFI_ACPI_6_1_IORT_ID_MAPPING;

typedef struct {
  UINT8        Type;
  UINT16       Length;
  UINT8        Revision;
  UINT32       Reserved_4;
  UINT32       NumberOfMappings;
  UINT32       IdArray;
  UINT64       MemoryAccessProperties;
  UINT32       AtsAttribute;
  UINT32       PciSegment;
  EFI_ACPI_6_1_IORT_ID_MAPPING  IdMapping[NUM_PCI_SEG];
} EFI_ACPI_6_1_IORT_RC_NODE;

typedef struct {
  UINT32       SmmuNSgIrpt;
  UINT32       SmmuNSgIrptFlags;
  UINT32       SmmuNSgCfgIrpt;
  UINT32       SmmuNSgCfgIprtFlags;
} EFI_ACPI_6_1_GLOBAL_INT_ARRAY;

typedef struct {
  UINT32       GSIV;
  UINT32       Flags;
} EFI_ACPI_6_1_INT_ARRAY;

typedef struct {
  UINT8        Type;
  UINT16       Length;
  UINT8        Revision;
  UINT32       Reserved_4;
  UINT32       NumberOfMappings;
  UINT32       IdArray;
  UINT64       BaseAddress;
  UINT64       Span;
  UINT32       Model;
  UINT32       Flags;
  UINT32       GlobalIntArray;
  UINT32       NumberOfContextInts;
  UINT32       ContextIntArray;
  UINT32       NumberOfPmuInts;
  UINT32       PmuIntArray;
  EFI_ACPI_6_1_GLOBAL_INT_ARRAY GlobalInts;
  EFI_ACPI_6_1_INT_ARRAY ContextInts[128];
  EFI_ACPI_6_1_INT_ARRAY PmuInts[3];
  EFI_ACPI_6_1_IORT_ID_MAPPING IdMapping[NUM_ITS];
} EFI_ACPI_6_1_IORT_SMMU_NODE;

typedef struct {
  UINT8        Type;
  UINT16       Length;
  UINT8        Revision;
  UINT32       Reserved_4;
  UINT32       NumberOfMappings;
  UINT32       IdArray;
  UINT32       NodeFlags;
  UINT64       MemoryAccessProperties;
  UINT8        DeviceMemoryAddressSize;
  UINT8        DeviceObjectName[32];
  UINT8        Padding[3];
  EFI_ACPI_6_1_IORT_ID_MAPPING  IdMapping[1];
} EFI_ACPI_6_1_IORT_NAMED_NODE;

#define IORT_CRYPTO_NAMED_NODE_OUTPUT_BASE    0x7f00
#define IORT_CRYPTO_NAMED_NODE_ID(pka, ring) \
    IORT_CRYPTO_NAMED_NODE_OUTPUT_BASE + CRYPTO_PKA_RING_DEVICE_ADR(pka, ring)

/* Defines Named Node entry information for crypto devices */
#define IORT_CRYPTO_NAMED_NODE_ENTRY(pka, ring)  \
  EFI_ACPI_6_1_IORT_TYPE_NAMED,             /* Type */                      \
  sizeof(EFI_ACPI_6_1_IORT_NAMED_NODE),     /* Length */                    \
  0,                                        /* Revision */                  \
  0,                                        /* Reserved_4 */                \
  1,                                        /* NumberOfMappings */          \
  OFFSET_OF(EFI_ACPI_6_1_IORT_NAMED_NODE, IdMapping), /* IdArray */         \
  0,                                        /* NodeFlags */                 \
  MEMORY_ACCESS_CCA | MEMORY_ACCESS_CPM,    /* MemoryAccessProperties */    \
  0,                                        /* DeviceMemoryAddressSize */   \
  CRYPTO_DEVICE_OBJECT_NAME(pka, ring),     /* DeviceObjectName */          \
  { 0 },                                    /* Padding */                   \
  {                                         /* IdMapping */                 \
    {                                                                       \
    0x0,                                    /* InputBase */                 \
    0x1,                                    /* NumberOfIds */               \
    IORT_CRYPTO_NAMED_NODE_ID(pka, ring), /* OutputBase */          \
    OFFSET_OF(EFI_ACPI_6_1_IORT_STRUCTURE, SmmuNode), /* OutputReference */ \
    0x1                                     /* Flags */                     \
    }                                                                       \
  }

#define NUM_PKA_DEV (NUM_CRYPTO_PKA_DEVICE * NUM_CRYPTO_PKA_RING_DEVICE)

typedef struct {
  EFI_ACPI_6_1_IORT_HEADER   Header;
  EFI_ACPI_6_1_IORT_ITS_NODE ItsNode;
  EFI_ACPI_6_1_IORT_RC_NODE  RcNode;
  EFI_ACPI_6_1_IORT_SMMU_NODE SmmuNode;
  EFI_ACPI_6_1_IORT_NAMED_NODE NamedNode[NUM_PKA_DEV];
} EFI_ACPI_6_1_IORT_STRUCTURE;

#pragma pack ()

#endif // __IORT_H__

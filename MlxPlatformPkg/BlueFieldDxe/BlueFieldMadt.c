#include "BlueFieldInternal.h"
#include "BlueFieldPlatform.h"
#include <Library/AcpiLib.h>
#include <Protocol/AcpiTable.h>
#include <IndustryStandard/Acpi.h>

#pragma pack (1)

//
// Multiple APIC Description Table
//
typedef struct {
  EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER   Header;
  //  EFI_ACPI_6_1_GIC_STRUCTURE                            GicInterfaces[FixedPcdGet32 (PcdCoreCount)];
  EFI_ACPI_6_1_GIC_STRUCTURE                            GicInterfaces[16];
  EFI_ACPI_6_1_GIC_DISTRIBUTOR_STRUCTURE                GicDistributor;
  EFI_ACPI_6_1_GICR_STRUCTURE                           GicRedistributor;
  EFI_ACPI_6_1_GIC_ITS_STRUCTURE                        GicITS;
} EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE;

#pragma pack ()

EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE Madt = {
  {
    MLNX_ACPI_HEADER (
      EFI_ACPI_1_0_APIC_SIGNATURE,
      EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE,
      EFI_ACPI_6_1_MULTIPLE_APIC_DESCRIPTION_TABLE_REVISION,
      EFI_ACPI_MLNX_OEM_TABLE_ID
    ),
    //
    // MADT specific fields
    //
    0, // LocalApicAddress
    0, // Flags
  },
  {
    // Format: EFI_ACPI_6_1_GICC_STRUCTURE_INIT(GicId, AcpiCpuUid, Mpidr,
    //    Flags, PmuIrq, GicBase, GicVBase, GicHBase, GsivId, GicRBase)
    // Note: The GIC Structure of the primary CPU must be the first entry (see note in 5.2.12.14 GICC Structure of
    //       ACPI v5.1).
    //       The cores from a same cluster are kept together. It is not an ACPI requirement but in case the OSPM uses
    //       the ACPI ARM Parking protocol, it might want to wake up the cores in the order of this table.
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(0, 0, GET_MPID(0, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(1, 1, GET_MPID(0, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(2, 2, GET_MPID(1, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(3, 3, GET_MPID(1, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(4, 4, GET_MPID(2, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(5, 5, GET_MPID(2, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(6, 6, GET_MPID(3, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(7, 7, GET_MPID(3, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(8, 8, GET_MPID(4, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(9, 9, GET_MPID(4, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(10, 10, GET_MPID(5, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(11, 11, GET_MPID(5, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(12, 12, GET_MPID(6, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(13, 13, GET_MPID(6, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(14, 14, GET_MPID(7, 0),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
    EFI_ACPI_6_1_GICC_STRUCTURE_INIT(15, 15, GET_MPID(7, 1),
        0, 23, FixedPcdGet32 (PcdGicInterruptInterfaceBase), 0x11020000,
        0x11010000, 25, FixedPcdGet32 (PcdGicRedistributorsBase)),
  },
  EFI_ACPI_6_1_GIC_DISTRIBUTOR_INIT(0, FixedPcdGet32 (PcdGicDistributorBase),
      0, EFI_ACPI_6_1_GIC_V3),
  EFI_ACPI_6_1_GICR_INIT(FixedPcdGet32 (PcdGicRedistributorsBase),
        FixedPcdGet32 (PcdGicRedistributorsLength)),
  EFI_ACPI_6_1_GIC_ITS_INIT(0, FixedPcdGet64 (PcdGicITSBase))
};

EFI_STATUS
BlueFieldAcpiInstallMadt (UINTN Cores, UINT64 CoreEnable)
{
  UINTN                         Index;
  EFI_STATUS                    Status;
  EFI_GUID                      Guid = EFI_ACPI_TABLE_PROTOCOL_GUID;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiProtocol;
  UINTN                         AcpiTableKey;

  DEBUG((EFI_D_ERROR, "BlueFieldAcpiInstallMadt\n"));

  Status = gBS->LocateProtocol (
                  &Guid,
                  NULL,
                  (VOID**)&AcpiProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG((EFI_D_ERROR, "BlueFieldAcpiInstallMadt: %d Cores\n", Cores));
  for (Index = 0; Index < Cores; Index++) {
    if (CoreEnable & (1 << Index))
      Madt.GicInterfaces[Index].Flags = EFI_ACPI_6_1_GIC_ENABLED;
  }

  Status = AcpiProtocol->InstallAcpiTable (
                       AcpiProtocol,
                       (EFI_ACPI_COMMON_HEADER *)&Madt,
                       sizeof (Madt),
                       &AcpiTableKey
                       );
  DEBUG((EFI_D_ERROR, "BlueFieldAcpiInstallMadt: Installed status=%d\n", Status));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

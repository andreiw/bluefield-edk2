/** @file
*  Secondary System Description Table Fields (SSDT)
*
*  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
**/

#include "BlueFieldPlatform.h"
#include "bf1_irqs.h"

#define PRT_ENTRY(Address, Pin, Interrupt)   \
          Package (4) {                      \
            Address,    /* uses the same format as _ADR */      \
            Pin,        /* The PCI pin number of the device */  \
            Zero,       /* allocated from the global interrupt pool. */ \
            Interrupt   /* global system interrupt number */            \
          }

/*
  See Reference [1] 6.1.1
  "High word–Device #, Low word–Function #. (for example, device 3, function 2 is
   0x00030002). To refer to all the functions on a device #, use a function number of FFFF)."
*/
#define ROOT_PRT_ENTRY(Pin, Interrupt)   PRT_ENTRY(0x0000FFFF, Pin, Interrupt)
                                                    // Device 0 for Bridge.


DefinitionBlock("SsdtPci.aml", "SSDT", 1, EFI_ACPI_MLNX_OEM_ID_STR, EFI_ACPI_MLNX_OEM_TABLE_ID_PCI_STR, EFI_ACPI_MLNX_OEM_REVISION) {
  Scope(_SB) {
    //
    // PCI Root Complex
    //
    Device(PCI0) {
      Name(_HID, EISAID("PNP0A08")) // PCI Express Root Bridge
      Name(_CID, EISAID("PNP0A03")) // Compatible PCI Root Bridge
      Name(_SEG, Zero) // PCI Segment Group number
      Name(_BBN, Zero) // PCI Base Bus Number
      Name(_CCA, 1) // Cache Coherency Attribute

      // Root Complex 0
      Device (RP0) {
        Name(_ADR, 0xF0000000)    // Dev 0, Func 0
      }

      // PCI Routing Table
      Name(_PRT, Package() {
        ROOT_PRT_ENTRY(0, BF1_TH_HCA0_LEVEL_INT0_INT),   // INTA
        ROOT_PRT_ENTRY(1, BF1_TH_HCA0_LEVEL_INT1_INT),   // INTB
        ROOT_PRT_ENTRY(2, BF1_TH_HCA0_LEVEL_INT2_INT),   // INTC
        ROOT_PRT_ENTRY(3, BF1_TH_HCA0_LEVEL_INT3_INT),   // INTD
      })

      // Root complex resources
      Method (_CRS, 0, Serialized) {
        Name (RBUF, ResourceTemplate () {
          WordBusNumber ( // Bus numbers assigned to this root
                ResourceProducer,
                MinFixed, MaxFixed, PosDecode,
                0,   // AddressGranularity
                0,   // AddressMinimum - Minimum Bus Number
                126, // AddressMaximum - Maximum Bus Number
                0,   // AddressTranslation - Set to 0
                127  // RangeLength - Number of Busses
          )

          QWordMemory ( // 32-bit BAR Windows
                ResourceProducer, PosDecode,
                MinFixed, MaxFixed,
                Cacheable, ReadWrite,
                0x00000000,   // Granularity
                0x00000000,   // Min Base Address
                0x7FFFFFFF,   // Max Base Address
                0xE000000000, // Translate
                0x80000000   // Length
          )

          QWordMemory ( // 64-bit BAR Windows
                ResourceProducer, PosDecode,
                MinFixed, MaxFixed,
                Prefetchable, ReadWrite,
                0x00000000,   // Granularity
                0xE200000000, // Min Base Address
                0xFFFFFFFFFF, // Max Base Address
                0x00000000,   // Translate
                0x1e00000000  // Length
          )
        }) // Name(RBUF)

        Return (RBUF)
      } // Method(_CRS)

      //
      // OS Control Handoff
      //
      Name(SUPP, 0x10) // PCI _OSC Support Field value
      Name(CTRL, Zero) // PCI _OSC Control Field value

      /*
       * See [1] 6.2.10, [2] 4.5
       */
       Method(_OSC,4) {
         // Check for the UUID of a pci host bridge device
         If(LEqual(Arg0,ToUUID("33DB4D5B-1FF7-401C-9657-7441C03DD766"))) {
         // Create DWord-adressable fields from the Capabilities Buffer
         CreateDWordField(Arg3,0,CDW1)
         CreateDWordField(Arg3,4,CDW2)
         CreateDWordField(Arg3,8,CDW3)

         // Save Capabilities DWord2 & 3
         Store(CDW2,SUPP)
         Store(CDW3,CTRL)

         // Only allow native hot plug control if OS supports:
         // * ASPM
         // * Clock PM
         // * MSI/MSI-X
         If(LNotEqual(And(SUPP, 0x16), 0x16)) {
           And(CTRL,0x1E,CTRL) // Mask bit 0 (and undefined bits)
         }

         // Always allow native PME, AER (no dependencies)

         // Never allow SHPC (no SHPC controller in this system)
         And(CTRL,0x1D,CTRL)

         If(LNotEqual(Arg1,One)) {      // Unknown revision
           Or(CDW1,0x08,CDW1)
         }

         If(LNotEqual(CDW3,CTRL)) {     // Capabilities bits were masked
           Or(CDW1,0x10,CDW1)
         }
         // Update DWORD3 in the buffer
         Store(CTRL,CDW3)
         Return(Arg3)
       } Else {
         Or(CDW1,4,CDW1) // Unrecognized UUID
         Return(Arg3)
       }
     } // End _OSC

     // Address space reserved for PCIe ECAM
     Device(RES0) {
       Name(_HID, EISAID("PNP0C02"))
       Name(_CRS, ResourceTemplate() {
         QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
           NonCacheable, ReadWrite,
           0,              // Granularity
           0xe100000000, // Range Minimum
           0xe10fffffff,  // Range Maximum
           0,             // Translation Offset
           0x0010000000,  // Length
         )
       })
     } // End RES0
    } // PCI0

    // TRIO0
    // TRIO0 is present in the mini model, but there are no practical
    // uses of it in the mini model, so we are putting it here.
    Device(TIO0) {
      Name(_HID, "MLNXBF06")
      Name(_UID, Zero)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
          NonCacheable, ReadWrite,
          0,             // Granularity
          0xdd00000000,  // Range Minimum
          0xdd000fffff,  // Range Maximum
          0,             // Translation Offset
          0x0000100000,  // Length
          ,, , AddressRangeMemory, TypeStatic)
        Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
        { BF1_TH_TP0_MAC_INT,
          BF1_TH_TP0_RSH_FULL_ERROR_INT,
          BF1_TH_TP0_MSG_Q_FULL_ERROR_INT,
          BF1_TH_TP0_MSG_Q_ARRIVED_INT,
          BF1_TH_TP0_MMIO_ERR_INT,
          BF1_TH_TP0_MAP_UNCLAIMED_INT,
          BF1_TH_TP0_RSH_SIZE_ERROR_INT,
          BF1_TH_TP0_PIO_ECAM_ERR_INT,
          BF1_TH_TP0_PIO_CPL_ERR_INT,
          BF1_TH_TP0_MMIO_PROT_ERR_INT,
          BF1_TH_TP0_PUSH_DMA_EVT_CTR_INT,
          BF1_TH_TP0_MAP_EVT_CTR_INT,
          BF1_TH_TP0_PIO_DISABLED_INT,
          BF1_TH_TP0_REM_MMIO_ERR_INT,
          BF1_TH_TP0_ERR_MSG_COR_INT,
          BF1_TH_TP0_ERR_MSG_NONFATAL_INT,
          BF1_TH_TP0_ERR_MSG_FATAL_INT }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "bus_number", Package () {"02:00.0"}},
          Package () { "num_irqs", Package () {17}},
        }
      })
    }

    // TRIO1
    Device(TIO1) {
      Name(_HID, "MLNXBF06")
      Name(_UID, 1)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
          NonCacheable, ReadWrite,
          0,             // Granularity
          0xdd40000000,  // Range Minimum
          0xdd400fffff,  // Range Maximum
          0,             // Translation Offset
          0x0000100000,  // Length
          ,, , AddressRangeMemory, TypeStatic)
        Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
        { BF1_TH_TP1_MAC_INT,
          BF1_TH_TP1_RSH_FULL_ERROR_INT,
          BF1_TH_TP1_MSG_Q_FULL_ERROR_INT,
          BF1_TH_TP1_MSG_Q_ARRIVED_INT,
          BF1_TH_TP1_MMIO_ERR_INT,
          BF1_TH_TP1_MAP_UNCLAIMED_INT,
          BF1_TH_TP1_RSH_SIZE_ERROR_INT,
          BF1_TH_TP1_PIO_ECAM_ERR_INT,
          BF1_TH_TP1_PIO_CPL_ERR_INT,
          BF1_TH_TP1_MMIO_PROT_ERR_INT,
          BF1_TH_TP1_PUSH_DMA_EVT_CTR_INT,
          BF1_TH_TP1_MAP_EVT_CTR_INT,
          BF1_TH_TP1_PIO_DISABLED_INT,
          BF1_TH_TP1_REM_MMIO_ERR_INT,
          BF1_TH_TP1_ERR_MSG_COR_INT,
          BF1_TH_TP1_ERR_MSG_NONFATAL_INT,
          BF1_TH_TP1_ERR_MSG_FATAL_INT }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "bus_number", Package () {"02:01.0"}},
          Package () { "num_irqs", Package () {17}},
        }
      })
    }

    // TRIO2
    Device(TIO2) {
      Name(_HID, "MLNXBF06")
      Name(_UID, 2)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
          NonCacheable, ReadWrite,
          0,             // Granularity
          0xdd80000000,  // Range Minimum
          0xdd800fffff,  // Range Maximum
          0,             // Translation Offset
          0x0000100000,  // Length
          ,, , AddressRangeMemory, TypeStatic)
        Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
        { BF1_TH_HCA0_MAC_INT,
          BF1_TH_HCA0_RSH_FULL_ERROR_INT,
          BF1_TH_HCA0_MSG_Q_FULL_ERROR_INT,
          BF1_TH_HCA0_MSG_Q_ARRIVED_INT,
          BF1_TH_HCA0_MMIO_ERR_INT,
          BF1_TH_HCA0_MAP_UNCLAIMED_INT,
          BF1_TH_HCA0_RSH_SIZE_ERROR_INT,
          BF1_TH_HCA0_PIO_ECAM_ERR_INT,
          BF1_TH_HCA0_PIO_CPL_ERR_INT,
          BF1_TH_HCA0_MMIO_PROT_ERR_INT,
          BF1_TH_HCA0_PUSH_DMA_EVT_CTR_INT,
          BF1_TH_HCA0_MAP_EVT_CTR_INT,
          BF1_TH_HCA0_PIO_DISABLED_INT,
          BF1_TH_HCA0_REM_MMIO_ERR_INT,
          BF1_TH_HCA0_ERR_MSG_COR_INT,
          BF1_TH_HCA0_ERR_MSG_NONFATAL_INT,
          BF1_TH_HCA0_ERR_MSG_FATAL_INT }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "bus_number", Package () {"02:02.0"}},
          Package () { "num_irqs", Package () {17}},
        }
      })
    }

    // MSS0 L3 cache
    // MSS0 is present in the mini model, but there are no practical
    // uses of it in the mini model, so we are putting it here.
    Device(MSS0) {
      Name(_HID, "MLNXBF07")
      Name(_UID, 0)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x18400000, 0x00000ad0) // MSS0 L3 0
        Memory32Fixed(ReadWrite, 0x18404000, 0x00000ad0) // MSS0 L3 1
      })
    }

    // MSS1 L3 cache
    Device(MSS1) {
      Name(_HID, "MLNXBF07")
      Name(_UID, 1)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x20400000, 0x00000ad0) // MSS1 L3 0
        Memory32Fixed(ReadWrite, 0x20404000, 0x00000ad0) // MSS1 L3 1
      })
    }
  }
}

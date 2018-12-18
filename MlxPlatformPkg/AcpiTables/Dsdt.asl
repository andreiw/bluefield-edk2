/** @file
*  Differentiated System Description Table Fields (DSDT)
*
*  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
**/

#include "BlueFieldPlatform.h"
#include "bf1_irqs.h"

// AMLFileName, TableSignature, ComplianceRevision, OEMID, TableID, OEMRevision)

DefinitionBlock("Dsdt.aml", "DSDT", 6, EFI_ACPI_MLNX_OEM_ID_STR, EFI_ACPI_MLNX_OEM_TABLE_ID_DSDT_STR, EFI_ACPI_MLNX_OEM_REVISION) {
  Scope(_SB) {
    //
    // Processor declaration
    //
    Device(CP00) { // A72-0: Cluster 0, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 0)
    }

    Device(CP01) { // A72-1: Cluster 0, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 1)
    }

    Device(CP02) { // A72-2: Cluster 1, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 2)
    }

    Device(CP03) { // A72-3: Cluster 1, Cpu 2
      Name(_HID, "ACPI0007")
      Name(_UID, 3)
    }

    Device(CP04) { // A72-4: Cluster 2, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 4)
    }

    Device(CP05) { // A72-5: Cluster 2, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 5)
    }

    Device(CP06) { // A72-6: Cluster 3, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 6)
    }

    Device(CP07) { // A72-7: Cluster 3, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 7)
    }

    Device(CP08) { // A72-8: Cluster 4, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 8)
    }

    Device(CP09) { // A72-9: Cluster 4, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 9)
    }

    Device(CP10) { // A72-10: Cluster 5, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 10)
    }

    Device(CP11) { // A72-11: Cluster 5, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 11)
    }

    Device(CP12) { // A72-12: Cluster 6, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 12)
    }

    Device(CP13) { // A72-13: Cluster 6, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 13)
    }

    Device(CP14) { // A72-14: Cluster 7, Cpu 0
      Name(_HID, "ACPI0007")
      Name(_UID, 14)
    }

    Device(CP15) { // A72-15: Cluster 7, Cpu 1
      Name(_HID, "ACPI0007")
      Name(_UID, 15)
    }

    // UART PL011
    Device(COM0) {
      Name(_HID, "ARMH0011")
      Name(_CID, "PL011")
      Name(_UID, Zero)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x1000000, 0x800000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive)
          { BF1_RSH0_DEVICE_UART0_INT }
      })
    }

    // UART PL011
    Device(COM1) {
      Name(_HID, "ARMH0011")
      Name(_CID, "PL011")
      Name(_UID, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x1800000, 0x800000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive)
          { BF1_RSH0_DEVICE_UART1_INT }
      })
    }

    // RShim TMFIFO
    Device(RSH0) {
      Name(_HID, "MLNXBF01")
      Name(_UID, Zero)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x00800a20, 0x00000018)
        Memory32Fixed(ReadWrite, 0x00800a40, 0x00000018)
        Interrupt(ResourceConsumer, Edge, ActiveHigh, Exclusive)
          { BF1_RSH0_TM_HTT_LWM_INT,
            BF1_RSH0_TM_HTT_HWM_INT,
            BF1_RSH0_TM_TTH_LWM_INT,
            BF1_RSH0_TM_TTH_HWM_INT
          }
      })
    }

    // GPIO Controller
    Device(GPI0) {
      Name(_HID, "MLNXBF02")
      Name(_UID, Zero)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x05800000, 0x00010000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive)
          { BF1_RSH0_DEVICE_GPIO_INT }
      })
    }

    // I2C SMBus Host Controller
    Device(I2C2) {
      Name(_HID, "MLNXBF03")
      Name(_UID, 2)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x02805000, 0x00000800)  // Smbus[2]
        Memory32Fixed(ReadWrite, 0x02801240, 0x00000020)  // CauseMaster[2]
        Memory32Fixed(ReadWrite, 0x028012a0, 0x00000020)  // CauseSlave[2]
        Memory32Fixed(ReadWrite, 0x02801300, 0x00000010)  // CauseCoalesce
        Memory32Fixed(ReadWrite, 0x02802000, 0x00000100)  // GPIO 0
        Memory32Fixed(ReadWrite, 0x02800358, 0x00000008)  // CorePll
        Interrupt(ResourceConsumer, Edge, ActiveHigh, Shared)
          { BF1_RSH0_DEVICE_TYU_INT }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "bus-freq", 100 },         // Smbus frequency in KHz
          Package () { "profile", "mlnx-bf18" },  // Device profile
        }
      })

        Device (SSIF) {
          Name(_HID, "IPI0001")
          Name(_UID, 0)
          Name(_STR, Unicode("IPMI_SSIF"))

          Method (_IFT)
          {
            Return(0x04)  // Return interface type for SSIF
          }

          Method (_ADR)
          {
            Return(0x20)  // Return SSIF Slave Address
          }

          Method (_SRV)
          {
            Return(0x0200) // IPMI Specification Version 2.0
          }
        }
    }

    // LiveFish
    Device(LVF0) {
      Name(_HID, "MLNXBF05")
      Name(_UID, Zero)
      Name(_CCA, 1)
      Name(_CRS, ResourceTemplate() {
        // 1MB of CR_SPACE forwarding through the TRIO mmio range
        QWordMemory(ResourceProducer, PosDecode, MinFixed, MaxFixed,
          NonCacheable, ReadWrite,
          0,             // Granularity
          0xdd80200000,  // Range Minimum
          0xdd802fffff,  // Range Maximum
          0,             // Translation Offset
          0x0000100000,  // Length
          ,, , AddressRangeMemory, TypeStatic)
        // The tyu.fuse area (CR space offset 0x1c1600) as mapped in the RSHIM
        Memory32Fixed(ReadWrite, 0x02801600, 0x00000080)
      })
    }

    // Firmware Controller
    Device(BFFW) {
      Name(_HID, "MLNXBF04")
      Name(_UID, Zero)
    }

    // eMMC controller
    Device (MMC0) {
      Name (_HID, "PRP0001")
      Name (_UID, Zero)
      Name (_CCA, 1)
      Name (_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x6008000, 0x400)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive)
          { BF1_RSH0_DEVICE_MMC_INT }
      })

      // Common configuration
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "compatible", Package () {"mellanox,bluefield-dw-mshc", "snps,dw-mshc"}},
          Package () { "fifo-depth", 0x100 },
          Package () { "clock-frequency", 24000000 },
          Package () { "cap-mmc-highspeed", 1 },
          Package () { "no-sdio", 1 },
          Package () { "no-sd", 1 },
          Package () { "broken-hpi", 1 },
          Package () { "card-detect-delay", 200 },
          Package () { "bus-width", 8 },
          Package () { "num-slots", 2 }
        }
      })

      // Slot-0
      Device (SLT0) {
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package () { "reg", 0 },              // physical slot number
            Package () { "bus-width", 8 }         // bus width (8-bit)
          }
        })
      }

      // Slot-1
      Device (SLT1) {
        Name(_DSD, Package() {
          ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
          Package() {
            Package () { "reg", 1 },              // physical slot number
            Package () { "bus-width", 4 }         // bus width (4-bit)
          }
        })
      }
    }

    // ARM SBSA Generic Watchdog
    // First memory region is control, second is refresh
    Device (WDG0) {
      Name (_HID, "PRP0001")
      Name (_UID, Zero)
      Name (_CCA, 1)
      Name (_CRS, ResourceTemplate() {
        Memory32Fixed(ReadWrite, 0x7800000, 0x1000)
        Memory32Fixed(ReadWrite, 0x7000000, 0x1000)
        Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive)
          { BF1_RSH0_DEVICE_WDOG_SEC_INT }
      })
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "compatible", Package () {"arm,sbsa-gwdt"}},
        }
      })
    }

    Device (PMC0) {
      Name (_HID, "MLNXBFD0")
      Name (_UID, 0)
      Name (_CCA, 1)
      Name(_DSD, Package() {
        ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
        Package() {
          Package () { "version", 2},
          Package () { "block_num", 21 },
          // Note that tile_num is patched at run-time based on core count
          Package () { "tile_num", 8 },
          Package () { "block_name", Package () { "mss0", "mss1", "ecc0", "ecc1", "trio0", "trio1", "trio2", "triogen0", "triogen1", "triogen2", "pcie0", "pcie1", "pcie2", "l3cache0half0", "l3cache0half1", "l3cache1half0", "l3cache1half1", "tile0", "tile1", "tile2", "tile3", "tile4", "tile5", "tile6", "tile7" }},
          // mss generic performance counters
          Package () { "mss0", Package () { 0x180000c0, 0x40, 4, 1 }},
          Package () { "mss1", Package () { 0x200000c0, 0x40, 4, 1 }},
          // mss ecc error statistics
          Package () { "ecc0", Package () {0x18408000, 0x360, 1, 0}},
          Package () { "ecc1", Package () {0x20408000, 0x360, 1, 0}},
          // trio performance counters
          Package () { "trio0", Package () { 0xdd000000c0, 0x40, 4, 1 }},
          Package () { "trio1", Package () { 0xdd400000c0, 0x40, 4, 1 }},
          Package () { "trio2", Package () { 0xdd800000c0, 0x40, 4, 1 }},
          // trio generic counters
          Package () { "triogen0", Package () { 0xdd00002598, 0x10, 1, 1 }},
          Package () { "triogen1", Package () { 0xdd40002598, 0x10, 1, 1 }},
          Package () { "triogen2", Package () { 0xdd80002598, 0x10, 1, 1 }},
          // pcie tlr statistics
          Package () { "pcie0", Package () { 0xdd00108000, 0x78, 1, 0 }},
          Package () { "pcie1", Package () { 0xdd40108000, 0x78, 1, 0 }},
          Package () { "pcie2", Package () { 0xdd80108000, 0x78, 1, 0 }},
          // l3 cache counters
          Package () { "l3cache0half0", Package () {0x18400980, 0x80, 5, 1}},
          Package () { "l3cache0half1", Package () {0x18404980, 0x80, 5, 1}},
          Package () { "l3cache1half0", Package () {0x20400980, 0x80, 5, 1}},
          Package () { "l3cache1half1", Package () {0x20404980, 0x80, 5, 1}},
          // hnf performance counters
          Package () { "tile0", Package () { 0x580400c0, 0x40, 4, 1 }},
          Package () { "tile1", Package () { 0x584400c0, 0x40, 4, 1 }},
          Package () { "tile2", Package () { 0x588400c0, 0x40, 4, 1 }},
          Package () { "tile3", Package () { 0x58c400c0, 0x40, 4, 1 }},
          Package () { "tile4", Package () { 0x590400c0, 0x40, 4, 1 }},
          Package () { "tile5", Package () { 0x594400c0, 0x40, 4, 1 }},
          Package () { "tile6", Package () { 0x598400c0, 0x40, 4, 1 }},
          Package () { "tile7", Package () { 0x59c400c0, 0x40, 4, 1 }},
        }
      })
    }
  } // Scope(_SB)
}

#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = BlueField
  PLATFORM_GUID                  = b6934d76-62d5-11e5-8797-001aca00bfc4
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/BlueField
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MlxPlatformPkg/BlueField.fdf

  SECURE_BOOT_ENABLE             = TRUE

!include StdLib/StdLib.inc
!include Mlx.dsc.inc

[LibraryClasses.common]
  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64Lib.inf
  ArmCpuLib|ArmPkg/Drivers/ArmCpuLib/ArmCortexAEMv8Lib/ArmCortexAEMv8Lib.inf
  ArmPlatformLib|MlxPlatformPkg/Library/BlueFieldLib/BlueFieldLib.inf

  ArmPlatformSysConfigLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressSysConfigLib/ArmVExpressSysConfigLib.inf

  LibStdLib|StdLib/LibC/StdLib/StdLib.inf

  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf

  DmaLib|ArmPkg/Library/ArmDmaLib/ArmDmaLib.inf

  BootParamFileSystemLib|MlxPlatformPkg/Filesystem/BootParam/BootParamFileSystem.inf

[LibraryClasses.common.SEC]
  ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64LibSec.inf
  ArmPlatformSecLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressSecLibRTSM/ArmVExpressSecLib.inf
  ArmPlatformLib|ArmPlatformPkg/ArmVExpressPkg/Library/ArmVExpressLibRTSM/ArmVExpressLibSec.inf

[LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.UEFI_APPLICATION, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.DXE_DRIVER]
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf

[BuildOptions]
  GCC:*_*_AARCH64_PLATFORM_FLAGS == -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include -I$(WORKSPACE)/ArmPlatformPkg/ArmVExpressPkg/Include/Platform/RTSM -I$(WORKSPACE)/MlxPlatformPkg/Include -march=armv8-a+crc
  *_*_*_ASL_FLAGS = -so


################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]

  ## If TRUE, Graphics Output Protocol will be installed on virtual handle created by ConsplitterDxe.
  #  It could be set FALSE to save size.
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE

[PcdsFixedAtBuild.common]
  gArmPlatformTokenSpaceGuid.PcdFirmwareVendor|"Mellanox BlueField"
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPrompt|"BlueField"

  # Up to 16 cores or 8 clusters
  gArmPlatformTokenSpaceGuid.PcdCoreCount|16
  gArmPlatformTokenSpaceGuid.PcdClusterCount|8

  gArmTokenSpaceGuid.PcdVFPEnabled|1

  # Stacks for MPCores in Secure World
  # Trusted SRAM (DRAM on Foundation model)
  gArmPlatformTokenSpaceGuid.PcdCPUCoresSecStackBase|0x00400000
  gArmPlatformTokenSpaceGuid.PcdCPUCoreSecPrimaryStackSize|0x1000
  gArmPlatformTokenSpaceGuid.PcdCPUCoreSecSecondaryStackSize|0x800

  # Stacks for MPCores in Normal World
  # Non-Trusted SRAM
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x2E000000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x4000
  gArmPlatformTokenSpaceGuid.PcdCPUCoreSecondaryStackSize|0x1000

  # System Memory (2GB - 16MB of Trusted DRAM at the top of the 32bit address space)
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x7F000000

  # Size of the region used by UEFI in permanent memory (Reserved 64MB)
  gArmPlatformTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x04000000

!if $(SECURE_BOOT_ENABLE) == TRUE
  # override the default values from SecurityPkg to ensure images from
  # all sources are verified in secure boot
  gEfiSecurityPkgTokenSpaceGuid.PcdOptionRomImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdFixedMediaImageVerificationPolicy|0x04
  gEfiSecurityPkgTokenSpaceGuid.PcdRemovableMediaImageVerificationPolicy|0x04
!endif

  #
  # ARM Pcds
  #
  gArmTokenSpaceGuid.PcdArmUncachedMemoryMask|0x0000000040000000

  ## Trustzone enable (to make the transition from EL3 to NS EL2 in ArmPlatformPkg/Sec)
  gArmTokenSpaceGuid.PcdTrustzoneSupport|TRUE

  #
  # ARM PrimeCell
  #

  ## PL011 - Serial Terminal
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200
  gArmPlatformTokenSpaceGuid.PL011UartClkInHz|156250000

  ## Variable Services
  # Note that the PcdVariableStoreSize must always be half of PcdEepromStoreSize
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000
  gEfiMdeModulePkgTokenSpaceGuid.PcdVariableStoreSize|0x8000

  # ARM Timer

  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|0x1d
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|0x1e
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|0x1b
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|0x1a

  #
  # ARM General Interrupt Controller
  #
!ifdef ARM_FVP_LEGACY_GICV2_LOCATION
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x11001000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x11002000
!else
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x12000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x12200000
  gArmTokenSpaceGuid.PcdGicRedistributorsLength|0x200000
  gArmTokenSpaceGuid.PcdGicInterruptInterfaceBase|0x11000000
  gArmTokenSpaceGuid.PcdGicITSBase|0x12020000
!endif

  #
  # ARM OS Loader
  #
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDescription|L"Linux from eMMC"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDevicePath|L"VenHw(8C91E049-9BF9-440E-BBAD-7DC5FC082C02)/Image"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootArgument|L"console=ttyAMA0 earlycon=pl011,0x01000000 initrd=initramfs"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootDtb|L"bf2.dtb"
  gArmPlatformTokenSpaceGuid.PcdDefaultBootAcpi|L"default"
  gArmPlatformTokenSpaceGuid.PcdBootParamPath|L"VenHw(F019E406-8C9C-11E5-8797-001ACA00BFC4)"
  # Set boot timeout
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|3

  # Use the serial console (ConIn & ConOut)
  gArmPlatformTokenSpaceGuid.PcdDefaultConOutPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi()"
  gArmPlatformTokenSpaceGuid.PcdDefaultConInPaths|L"VenHw(D3987D4B-971A-435F-8CAF-4967EB627241)/Uart(115200,8,N,1)/VenPcAnsi()"

  #
  # ARM Architectural Timer Frequency
  #
  # Set tick frequency value to 100Mhz
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|100000000

  #
  # I2C SMBus
  #
  # Note that the PcdVariableStoreSize must always be half of PcdEepromStoreSize
  gMlxPlatformTokenSpaceGuid.PcdI2cSmbusBitmask|0x5
  gMlxPlatformTokenSpaceGuid.PcdI2cSmbusFrequencyKhz|100
  gMlxPlatformTokenSpaceGuid.PcdEepromStoreAddress|0x57
  gMlxPlatformTokenSpaceGuid.PcdEepromStoreSize|0x10000
  gMlxPlatformTokenSpaceGuid.PcdEepromStorePageSize|0x80
  gMlxPlatformTokenSpaceGuid.PcdEepromBusId|0
  gMlxPlatformTokenSpaceGuid.PcdRtcAddress|0x68
  gMlxPlatformTokenSpaceGuid.PcdRtcBusId|0
  gMlxPlatformTokenSpaceGuid.PcdIpmbBmcAddress|0x20
  gMlxPlatformTokenSpaceGuid.PcdIpmbSlaveAddress|0x30
  gMlxPlatformTokenSpaceGuid.PcdIpmbBusId|2
  gMlxPlatformTokenSpaceGuid.PcdIpmbRetryCnt|1

[PcdsDynamicDefault.common]
  #
  # The size of a dynamic PCD of the (VOID*) type can not be increased at run
  # time from its size at build time. Set the "PcdFdtDevicePaths" PCD to a 128
  # character "empty" string, to allow to be able to set FDT text device paths
  # up to 128 characters long.
  #
  gEmbeddedTokenSpaceGuid.PcdFdtDevicePaths|L"                                                                                                                                "

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]

  #
  # SEC
  #
  ArmPlatformPkg/Sec/Sec.inf {
    <LibraryClasses>
      # Use the implementation which set the Secure bits
      ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicSecLib.inf
  }

  #
  # PEI Phase modules
  #
  # UEFI is placed in RAM by bootloader
  ArmPlatformPkg/PrePi/PeiMPCore.inf {
    <LibraryClasses>
      ArmLib|ArmPkg/Library/ArmLib/AArch64/AArch64Lib.inf
      ArmPlatformLib|MlxPlatformPkg/Library/BlueFieldLib/BlueFieldLib.inf
      ArmPlatformGlobalVariableLib|ArmPlatformPkg/Library/ArmPlatformGlobalVariableLib/PrePi/PrePiArmPlatformGlobalVariableLib.inf
  }
  MdeModulePkg/Universal/Variable/Pei/VariablePei.inf

  #
  # DXE
  #
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
  }

  #
  # Architectural Protocols
  #
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  MlxPlatformPkg/Variable/EmuVariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
  }
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
  }
  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
!else
  MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf {
    <LibraryClasses>
      NULL|MdeModulePkg/Library/VarCheckUefiLib/VarCheckUefiLib.inf
  }
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
!endif
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/FaultTolerantWriteDxe/FaultTolerantWriteDxe.inf
  MdeModulePkg/Universal/MonotonicCounterRuntimeDxe/MonotonicCounterRuntimeDxe.inf
  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  MlxPlatformPkg/RealTimeClock/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf

  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  EmbeddedPkg/SerialDxe/SerialDxe.inf

  #
  # ACPI Support
  #

  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MlxPlatformPkg/AcpiTables/AcpiTables.inf
  MlxPlatformPkg/AcpiTables/NoTrioTables.inf

  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf

  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf

  #
  # Multimedia Card Interface
  #
  EmbeddedPkg/Universal/MmcDxe/MmcDxe.inf
  MlxPlatformPkg/Drivers/DWMciDxe/DWMciDxe.inf

  #
  # TmFIFO Driver
  #
  MlxPlatformPkg/Drivers/TmFifoDxe/TmFifoDxe.inf

  #
  # PCI support
  #
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MlxPlatformPkg/Drivers/PciHostBridge/PciHostBridgeDxe.inf
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #
  # SATA.
  #
  MdeModulePkg/Bus/Ata/AtaAtapiPassThru/AtaAtapiPassThru.inf
  MdeModulePkg/Bus/Ata/AtaBusDxe/AtaBusDxe.inf
  MdeModulePkg/Bus/Pci/SataControllerDxe/SataControllerDxe.inf

  #
  # BlueField boot stream filesystem
  #
  MlxPlatformPkg/Filesystem/BfbFs/BfbFs.inf

  #
  # Platform Driver
  #
  MlxPlatformPkg/BlueFieldDxe/BlueFieldDxe.inf
  MlxPlatformPkg/SysConfigDxe/SysConfigDxe.inf

  #
  # SMBIOS
  #
  MlxPlatformPkg/PlatformSmbiosDxe/PlatformSmbiosDxe.inf
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf

  #
  # USB support (i.e. for plug-in xHCI host).
  #
  MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf

  #
  # iSCSI
  #
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf

  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  #
  # Bds
  #
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/Disk/RamDiskDxe/RamDiskDxe.inf
  IntelFrameworkModulePkg/Universal/BdsDxe/BdsDxe.inf
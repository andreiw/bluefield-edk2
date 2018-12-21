/** @file
*
*  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
*  Copyright (c) 2017, Mellanox Technologies. All rights reserved.
*
*  This program and the accompanying materials are licensed and made
*  available under the terms and conditions of the BSD License which
*  accompanies this distribution.  The full text of the license may be
*  found at http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS"
*  BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER
*  EXPRESS OR IMPLIED.
*
**/

#include "BdsPlatform.h"
#include "BlueFieldPlatform.h"
#include <Library/SortLib.h>
#include <Library/PrintLib.h>
#include <Protocol/RamDisk.h>

typedef UINT8 MLNX_EFI_BOOT_SETUP;

#define MLNX_BOOT_SETUP_INFO_BYTE  10
#define MLNX_BOOT_SETUP_INFO  \
        (*((UINT8 *) TILE_UEFI_IMAGE_ADDR + MLNX_BOOT_SETUP_INFO_BYTE))

#define MLNX_BOOT_SETUP_MASK 0x0002
#define MLNX_BOOT_SETUP_FLAG \
        ((MLNX_BOOT_SETUP_INFO & MLNX_BOOT_SETUP_MASK) >> 1)

//
// External boot setup.  This is similar to 'BootNext' configuration
// and aims to setup the boot option to use to load the OS.  This might
// be used to boot from Rshim USB.
//
#define MLNX_EXTERNAL_BOOT_SETUP     1
//
// Default boot setup. This is the normal boot, so use the existing
// boot order and the associated boot options.  At first boot, a boot
// option is created and/or boot devices are enumerated.
//
#define MLNX_DEFAULT_BOOT_SETUP      0


static CHAR16 *RamCDDesc = L"Virtual CD";
static CHAR16 *RamDiskDesc = L"Virtual Disk";

/**
  Get boot setup by looking up the external boot flag in Mellanox-based
  platform information.

  @param  BootSetup             Boot setup from external boot flag.

  @retval EFI_SUCCESS           Successfully get boot mode

**/
EFI_STATUS
MlnxGetBootSetup (
  OUT MLNX_EFI_BOOT_SETUP    *BootSetup
  )
{
  *BootSetup = MLNX_BOOT_SETUP_FLAG;
  return EFI_SUCCESS;
}

/**
  Clean any RAM disk boot options.

**/
STATIC
VOID
CleanRamDiskBootOptions (
  VOID
  )
{
  UINTN Index;
  UINT16 *BootOrder;
  UINT16 BootOption[10];
  UINTN BootOrderSize;
  UINTN BootOptionSize;
  UINT8 *BootOptionVar;
  EFI_STATUS Status;
  CHAR16 *Desc;

  BootOrder = BdsLibGetVariableAndSize (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                &BootOrderSize
                );
  if (BootOrder == NULL) {
    return;
  }

  for (Index = 0; Index < BootOrderSize / sizeof (UINT16); Index++ ) {
    UnicodeSPrint (BootOption, sizeof (BootOption), L"Boot%04x", BootOrder[Index]);
    BootOptionVar = BdsLibGetVariableAndSize (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      &BootOptionSize
                      );
    /*
     * Ewww.. this is hard-coded all-over the place.
     */
    Desc = (VOID *)(BootOptionVar + sizeof (UINT32) + sizeof (UINT16));
    if (BootOptionVar == NULL ||
        /*
         * Could have/should have checked the device path, but meh.
         */
        StringCompare (&Desc, &RamCDDesc) == 0 ||
        StringCompare (&Desc, &RamDiskDesc) == 0
        ) {
      if (BootOptionVar != NULL) {
        DEBUG ((EFI_D_INFO, "Deleting '%s'\n", Desc));
      }
      BdsDeleteBootOption (
        BootOrder[Index],
        BootOrder,
        &BootOrderSize
        );
    }

    FreePool (BootOptionVar);
  }

  Status = gRT->SetVariable (
    L"BootOrder",
    &gEfiGlobalVariableGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    BootOrderSize,
    BootOrder
    );

  //
  // Shrinking variable with existing variable implementation shouldn't fail.
  //
  ASSERT_EFI_ERROR (Status);

  FreePool (BootOrder);
}


/**
  Load a RAM disk, and return the device path to it.

  @param IsCd                    True if RAM disk is a virtual CD.

**/
STATIC
EFI_DEVICE_PATH *
RegisterRamDisk (
  IN  BOOLEAN IsCd
  )
{
  EFI_STATUS Status;
  EFI_RAM_DISK_PROTOCOL *RamDisk;
  EFI_DEVICE_PATH *Path;
  VOID *Buffer;
  UINTN Size;

  Status = gBS->LocateProtocol (&gEfiRamDiskProtocolGuid, NULL, (VOID **) &RamDisk);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: no RAM Disk Protocol: %r\n", __FILE__, Status));
    return NULL;
  }

  DEBUG((EFI_D_INFO, "Loading RAM disk...\n"));
  Buffer = BootParamFileSystemGetBuffer("ramdisk", NULL, &Size);
  if (Buffer == NULL) {
    DEBUG ((EFI_D_ERROR, "%a: couldn't load RAM disk\n", __FILE__));
    return NULL;
  }

  Status = RamDisk->Register (
             (UINTN) Buffer, Size, IsCd ?
             &gEfiVirtualCdGuid : &gEfiVirtualDiskGuid,
             NULL, &Path
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "%a: couldn't register RAM disk: %r\n",
            __FILE__, Status));
    FreePool (Buffer);
    return NULL;
  }

  return Path;
}

/**
  Create a boot option based on the PcdDefault and the boot parameter
  file system.

  @param BootOptionList          The header of the boot option linked list, to
                                 which the new boot option will be attached.

**/
EFI_STATUS
CreateDefaultBootOption (
  IN LIST_ENTRY                      *BootOptionList
  )
{
  EFI_STATUS                          Status;
  EFI_BOOT_MODE                       BootMode;
  MLNX_EFI_BOOT_SETUP                 BootSetup;
  EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL* EfiDevicePathFromTextProtocol;
  CHAR16*                             BootDescription;
  CHAR16*                             BootDevicePathText;
  CHAR16*                             BootArgument;
  EFI_DEVICE_PATH*                    BootDevicePath = NULL;
  CHAR16*                             DoRamDisk = L"ramdisk";
  CHAR16*                             DoRamCD = L"ramcd";

  BootDescription    = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootDescription);
  BootDevicePathText = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootDevicePath);
  BootArgument       = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootArgument);

  Status = gBS->LocateProtocol (&gEfiDevicePathFromTextProtocolGuid, NULL,
                                (VOID **)&EfiDevicePathFromTextProtocol);
  ASSERT_EFI_ERROR (Status);

  CleanRamDiskBootOptions();

  DEBUG ((EFI_D_INFO, "CreateDefaultBootOption %s\n", BootDevicePathText));

  if (StringCompare (&BootDevicePathText, &DoRamDisk) == 0) {
    //
    // Mount the RAM disk and boot from it.
    //
    BootDevicePath = RegisterRamDisk(FALSE);
    BootDescription = RamDiskDesc;
  } else if (StringCompare (&BootDevicePathText, &DoRamCD) == 0) {
    //
    // Mount the RAM virtual CD and boot from it.
    //
    BootDevicePath = RegisterRamDisk(TRUE);
    BootDescription = RamCDDesc;
  } else {
    BootDevicePath = EfiDevicePathFromTextProtocol->ConvertTextToDevicePath (BootDevicePathText);
  }

  if (BootDevicePath == NULL) {
    return EFI_UNSUPPORTED;
  }

  if (BootArgument != NULL && StrLen (BootArgument) == 0) {
    BootArgument = NULL;
  }

  //
  // Get current Boot Mode (Optional); might be used in the future.
  //
  Status = BdsLibGetBootMode (&BootMode);
  DEBUG ((EFI_D_INFO, "Boot Mode:%x\n", BootMode));

  //
  // Get current Boot Setup.
  //
  Status = MlnxGetBootSetup (&BootSetup);
  DEBUG ((EFI_D_INFO, "Boot Setup:%x\n", BootSetup));

  switch (BootSetup) {

  case MLNX_EXTERNAL_BOOT_SETUP:
    //
    // Create the default boot option.  If the boot option exists then
    // the boot order is updated. The boot option order is set to zero.
    //
    return BdsLibRegisterDefaultOption (BootOptionList,
                                        BootDevicePath,
                                        BootDescription,
                                        BootArgument,
                                        L"BootOrder");

  case MLNX_DEFAULT_BOOT_SETUP:
    //
    // Parse the boot order to get boot option
    //
    BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
    //
    // Register the new boot option.
    //
    BdsLibRegisterNewOption (BootOptionList,
                             BootDevicePath,
                             BootDescription,
                             BootArgument,
                             L"BootOrder");
    //
    // For now, no additional platform behavior is required. So fall
    // through.
    //
  default:
    //
    // The BootOrder variable may have changed, reload the in-memory list
    // with it. The following code only do the EnumerateAll at first boot.
    //
    BdsLibBuildOptionFromVar (BootOptionList, L"BootOrder");
    if (IsListEmpty (BootOptionList)) {
      //
      // If cannot find "BootOrder" variable, it may be first boot and
      // default boot option is empty. Try to enumerate all boot options
      // here.
      //
      BdsLibEnumerateAllBootOption (BootOptionList);
    }
  }

  return EFI_SUCCESS;
}

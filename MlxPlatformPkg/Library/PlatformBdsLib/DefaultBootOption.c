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
  EFI_DEVICE_PATH*                    BootDevicePath;

  BootDescription    = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootDescription);
  BootDevicePathText = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootDevicePath);
  BootArgument       = (CHAR16 *)BPO_PcdGetPtr (PcdDefaultBootArgument);

  Status = gBS->LocateProtocol (&gEfiDevicePathFromTextProtocolGuid, NULL,
                                (VOID **)&EfiDevicePathFromTextProtocol);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "CreateDefaultBootOption %s\n", BootDevicePathText));

  BootDevicePath = EfiDevicePathFromTextProtocol->ConvertTextToDevicePath (BootDevicePathText);

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

/** @file

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BlueFieldSysConfig.h"
#include "../../SysConfigDxe/SysConfig.h"

//
// Configuration Device Path
//
MLX_SYS_CONFIG_HII_VENDOR_DEVICE_PATH(mHiiVendorDevicePath);

VOID
EFIAPI
PlatformBdsAuth (
  VOID
  )
{
  EFI_HANDLE                           *Handle;
  EFI_DEVICE_PATH                      *Path;
  SYS_CONFIG_PRIVATE_DATA              *SysCfg;
  EFI_HII_CONFIG_ACCESS_PROTOCOL       *Interface;
  EFI_STATUS                           Status;

  Path = (EFI_DEVICE_PATH *)&mHiiVendorDevicePath;

  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &Path, (VOID **) &Handle);
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = gBS->HandleProtocol(Handle, &gEfiHiiConfigAccessProtocolGuid, (VOID **) &Interface);
  if (EFI_ERROR (Status)) {
    return;
  }

  SysCfg = SYS_CONFIG_PRIVATE_FROM_THIS(Interface);

  if (SysCfg->AuthCheck != NULL) {
    SysCfg->AuthCheck(Interface);
  }
}

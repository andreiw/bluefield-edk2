/** @file

  Copyright (c) 2013-2015, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BlueFieldInternal.h"

#include <PiDxe.h>
#include <Simulator.h>

#include <Library/AcpiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmShellCmdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/FirmwareVolume2.h>

/* Sys Config. */
SYS_CONFIG MlnxSysConfig;

/* Set to 1 to use device tree, 0 for ACPI */
#define USE_FDT 0

STATIC
EFI_STATUS
FindFdtByGuid (
  IN OUT   EFI_DEVICE_PATH  **FdtDevicePath,
  IN CONST EFI_GUID         *FdtGuid
  )
{
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH    FileDevicePath;
  EFI_HANDLE                           *HandleBuffer;
  UINTN                                HandleCount;
  UINTN                                Index;
  EFI_FIRMWARE_VOLUME2_PROTOCOL        *FvProtocol;
  EFI_GUID                             NameGuid;
  UINTN                                Size;
  VOID                                 *Key;
  EFI_FV_FILETYPE                      FileType;
  EFI_FV_FILE_ATTRIBUTES               Attributes;
  EFI_DEVICE_PATH                      *FvDevicePath;
  EFI_STATUS                           Status;

  if (FdtGuid == NULL) {
    return EFI_NOT_FOUND;
  }

  EfiInitializeFwVolDevicepathNode (&FileDevicePath, FdtGuid);

  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (VOID **) &FvProtocol
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    // Allocate Key
    Key = AllocatePool (FvProtocol->KeySize);
    ASSERT (Key != NULL);
    ZeroMem (Key, FvProtocol->KeySize);

    do {
      FileType = EFI_FV_FILETYPE_RAW;
      Status = FvProtocol->GetNextFile (FvProtocol, Key, &FileType, &NameGuid, &Attributes, &Size);
      if (Status == EFI_NOT_FOUND) {
        break;
      }
      if (EFI_ERROR (Status)) {
        return Status;
      }

      //
      // Check whether this file is the one we are looking for. If so,
      // create a device path for it and return it to the caller.
      //
      if (CompareGuid (&NameGuid, FdtGuid)) {
          Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID **)&FvDevicePath);
          if (!EFI_ERROR (Status)) {
            *FdtDevicePath = AppendDevicePathNode (FvDevicePath,
                               (EFI_DEVICE_PATH_PROTOCOL *)&FileDevicePath);
          }
          goto Done;
      }
    } while (TRUE);
    FreePool (Key);
  }

  if (Index == HandleCount) {
    Status = EFI_NOT_FOUND;
  }
  return Status;

Done:
  FreePool (Key);
  return Status;
}

/**
 * Generic UEFI Entrypoint for 'BlueFieldDxe' driver
 * See UEFI specification for the details of the parameters
 */
EFI_STATUS
EFIAPI
BlueFieldInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                   Status;
  CONST MLNX_PLATFORM          *Platform;
  EFI_DEVICE_PATH              *FdtDevicePath;
  CHAR16                       *TextDevicePath;
  UINTN                        TextDevicePathSize;
  VOID                         *Buffer;
  CONST MLNX_PLATFORM          *AcpiTable;
  UINTN                        BufferSize;

  Status = MlnxGetPlatform (&Platform);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Read SysConfig and use default zero values if not exist.
  gRT->GetVariable (SYS_CONFIG_VAR, &gMlxSysConfigGuid, NULL,
                    &BufferSize, &MlnxSysConfig);

  Status = MlnxGetAcpi(&AcpiTable);
  if (Status == EFI_NOT_FOUND) {
    FdtDevicePath = NULL;
    Status = FindFdtByGuid (&FdtDevicePath, Platform->Guid);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    TextDevicePath = ConvertDevicePathToText (FdtDevicePath, FALSE, FALSE);
    if (TextDevicePath != NULL) {
      TextDevicePathSize = StrSize (TextDevicePath);

      Buffer = PcdSetPtr (PcdFdtDevicePaths, &TextDevicePathSize, TextDevicePath);
      if (Buffer == NULL) {
        DEBUG ((EFI_D_ERROR,
                "BlueFieldDxe: Setting of FDT device path in PcdFdtDevicePaths failed - %r\n",
                EFI_BUFFER_TOO_SMALL));
      }
      FreePool (TextDevicePath);
    }
    FreePool (FdtDevicePath);
  }
  else
  {
    Status = LocateAndInstallAcpiFromFvConditional(AcpiTable->Guid,
                                                   BlueFieldAcpiCheck);

    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BlueFieldDxe: install acpi table:%d\n", Status));
    }
    Status = BlueFieldAcpiInstallNvdimm();
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "BlueFieldDxe: install NVDIMM table:%d\n", Status));
    }
  }

  // Install dynamic Shell command to run baremetal binaries.
  Status = ShellDynCmdRunAxfInstall (ImageHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "BlueFieldDxe: Failed to install ShellDynCmdRunAxf\n"));
  }

  return Status;
}

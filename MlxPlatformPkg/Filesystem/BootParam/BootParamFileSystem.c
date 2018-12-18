/** @file
*  Support a file system for passing boot parameters through the
*  BlueField boot stream.
*
*  Copyright (c) 2016, Mellanox Technologies. All rights reserved.
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

#include <BootParamFileSystem.h>

/**
  Look up a provided name in the boot parameter filesystem, after
  removing a prefix.  If the file is present, return a pointer to a
  string with the ASCII contents of the file, which will have been
  allocated from the pool.  If not, return NULL.

  @param EntryName   Name to look up.
  @param Prefix      Prefix to remove.
  @return A pointer as described above.

**/
STATIC
CHAR8 *
BootParamFileSystemGetEntry (
  IN CONST CHAR8 *EntryName,
  IN CONST CHAR8 *Prefix
  )
{
  CHAR8 *Result = NULL;

  EFI_STATUS Status;
  EFI_HANDLE Handle;

  const CHAR16 *BootParamPath = PcdGetPtr (PcdBootParamPath);

  if (BootParamPath == NULL || *BootParamPath == '\0') {
    return Result;
  }

  EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *EfiDevicePathFromTextProtocol;

  Status = gBS->LocateProtocol (&gEfiDevicePathFromTextProtocolGuid, NULL,
                                (VOID **)&EfiDevicePathFromTextProtocol);
  if (EFI_ERROR (Status))
    return Result;

  EFI_DEVICE_PATH *DevicePath;
  DevicePath = EfiDevicePathFromTextProtocol->ConvertTextToDevicePath (BootParamPath);
  if (DevicePath == NULL) {
    return Result;
  }

  Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &DevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    return Result;
  }

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FsProtocol;
  Status = gBS->OpenProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid,
                              (VOID**)&FsProtocol, gImageHandle, Handle,
                              EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (Status)) {
    return Result;
  }

  // Open the volume and get the root directory
  EFI_FILE_PROTOCOL *Fs;
  Status = FsProtocol->OpenVolume (FsProtocol, &Fs);
  if (!EFI_ERROR (Status))
  {
    EFI_FILE_PROTOCOL *File;
    UINTN PrefixLen = AsciiStrLen (Prefix);

    if (!AsciiStrnCmp (EntryName, Prefix, PrefixLen)) {
      EntryName += PrefixLen;
    }

    // Convert the name to Unicode
    CHAR16 UnicodeName[AsciiStrLen (EntryName) + 1];
    AsciiStrToUnicodeStr (EntryName, UnicodeName);

    Status = Fs->Open (Fs, &File, UnicodeName, EFI_FILE_MODE_READ, 0);
    if (!EFI_ERROR (Status)) {
      // Figure out how big our FileInfo buffer needs to be, then allocate it
      EFI_FILE_INFO *FileInfo = NULL;
      UINTN InfoSize = 0;
      if (File->GetInfo (File, &gEfiFileInfoGuid, &InfoSize, NULL) ==
          EFI_BUFFER_TOO_SMALL) {
        FileInfo = AllocatePool (InfoSize);
      }
      if (FileInfo) {
        // Get the info so we can get the file size
        Status = File->GetInfo (File, &gEfiFileInfoGuid, &InfoSize, FileInfo);
        if (!EFI_ERROR (Status)) {
          UINT64 Size = FileInfo->FileSize;
          //
          // Allocate our return value
          //
          CHAR8 *ReturnBuf = AllocatePool (Size + 1);
          if (ReturnBuf != NULL) {
            Status = File->Read (File, &Size, (VOID*)ReturnBuf);
            if (!EFI_ERROR (Status)) {
              //
              // Null-terminate and remove trailing newline(s).
              //
              ReturnBuf[Size] = '\0';
              while (--Size >= 0 && ReturnBuf[Size] == '\n')
                ReturnBuf[Size] = '\0';

              Result = ReturnBuf;
            }
          }
          FreePool (FileInfo);
        }
      }
      File->Close (File);
    }
  }

  gBS->CloseProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid,
         gImageHandle, Handle);

  return Result;
}

/**
  Look up a provided name in the boot parameter filesystem, after
  removing an optional "PcdDefault" prefix.  If the file is present,
  return a pointer to a Unicode string derived from the ASCII contents
  of the file.  If not, return the given default pointer.  Note that,
  if the returned pointer is not the provided default, the string it
  points to was allocated from the pool.  (We expect that we'll end up
  leaking this, since the code that calls this doesn't expect to get
  dynamically allocated data, but that shouldn't be a big deal.)

  @param EntryName   Name to look up.
  @param DefaultPtr  Pointer to return if we could not find or read the
                     specified file.
  @return A pointer as described above.

**/
CONST VOID *
_BPO_PcdGetPtr (
  IN CONST CHAR8 *EntryName,
  IN CONST VOID  *DefaultPtr
  )
{
  CHAR8 *BPStr = BootParamFileSystemGetEntry (EntryName, "PcdDefault");
  CONST VOID *Result = DefaultPtr;

  if (BPStr != NULL) {
    // Convert to Unicode.
    UINTN len = AsciiStrLen (BPStr);
    CHAR16 *UniBPStr = AllocatePool (sizeof (CHAR16) * (len + 1));
    if (UniBPStr != NULL) {
      AsciiStrToUnicodeStr (BPStr, UniBPStr);
      Result = UniBPStr;
    }
    FreePool (BPStr);
  }

  return Result;
}

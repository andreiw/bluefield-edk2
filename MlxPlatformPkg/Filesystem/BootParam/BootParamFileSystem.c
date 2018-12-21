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
#include <Library/DebugLib.h>

/**
  Look up a provided name in the boot parameter filesystem, after
  removing an optional prefix.  If the file is present, return a
  pointer to the opened file. If not, return NULL.

  @param EntryName   Name to look up.
  @param Prefix      Prefix to remove.
  @return A pointer as described above.

**/
STATIC
EFI_FILE_PROTOCOL *
BootParamFileSystemOpen (
  IN CONST CHAR8 *EntryName,
  IN CONST CHAR8 *Prefix
  )
{
  UINTN PrefixLen;
  EFI_STATUS Status;
  EFI_HANDLE Handle;
  EFI_FILE_PROTOCOL *Fs;
  EFI_FILE_PROTOCOL *File;
  EFI_DEVICE_PATH *DevicePath;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FsProtocol;
  CHAR16 UnicodeName[AsciiStrLen (EntryName) + 1];
  EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *EfiDevicePathFromTextProtocol;
  const CHAR16 *BootParamPath = PcdGetPtr (PcdBootParamPath);

  if (BootParamPath == NULL || *BootParamPath == '\0') {
    return NULL;
  }

  Status = gBS->LocateProtocol (&gEfiDevicePathFromTextProtocolGuid, NULL,
                                (VOID **)&EfiDevicePathFromTextProtocol);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  DevicePath = EfiDevicePathFromTextProtocol->ConvertTextToDevicePath (BootParamPath);
  if (DevicePath == NULL) {
    return NULL;
  }

  Status = gBS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, &DevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Status = gBS->OpenProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid,
                              (VOID**)&FsProtocol, gImageHandle, Handle,
                              EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  // Open the volume and get the root directory
  Status = FsProtocol->OpenVolume (FsProtocol, &Fs);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  if (Prefix != NULL) {
    PrefixLen = AsciiStrLen (Prefix);
    if (!AsciiStrnCmp (EntryName, Prefix, PrefixLen)) {
      EntryName += PrefixLen;
    }
  }

  // Convert the name to Unicode
  AsciiStrToUnicodeStr (EntryName, UnicodeName);

  Status = Fs->Open (Fs, &File, UnicodeName, EFI_FILE_MODE_READ, 0);
  Fs->Close (Fs);

  gBS->CloseProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid,
                      gImageHandle, Handle);

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return File;
}

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
  EFI_FILE_PROTOCOL *File = BootParamFileSystemOpen (EntryName, Prefix);

  if (File != NULL) {
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

  return Result;
}

/**
  Look up a provided name in the boot parameter filesystem, after
  removing an optional prefix. If the file is present, return a pointer
  to a buffer with the contents of the file, which will have been
  allocated from the pool. If not, return NULL.

  @param EntryName   Name to look up.
  @param Prefix      Prefix to remove.
  @param OutSize     Pointer to store the size.
  @return A pointer as described above.

**/
CHAR8 *
BootParamFileSystemGetBuffer (
  IN  CONST CHAR8 *EntryName,
  IN  CONST CHAR8 *Prefix,
  OUT UINTN *OutSize
  )
{
  CHAR8 *Result = NULL;

  EFI_STATUS Status;
  EFI_FILE_PROTOCOL *File = BootParamFileSystemOpen (EntryName, Prefix);

  if (File != NULL) {
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
        CHAR8 *ReturnBuf = AllocatePool (Size);
        if (ReturnBuf != NULL) {
          Status = File->Read (File, &Size, (VOID*)ReturnBuf);
          if (!EFI_ERROR (Status)) {
            *OutSize = Size;
            Result = ReturnBuf;
          }
        }
        FreePool (FileInfo);
      }
    }
    File->Close (File);
  }

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

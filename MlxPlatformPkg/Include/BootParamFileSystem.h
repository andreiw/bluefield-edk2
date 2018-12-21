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

#include <Uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/SimpleFileSystem.h>

/**
  Look up a provided name in the boot parameter filesystem, after
  removing an optional prefix.  If the file is present, return a pointer
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
  );

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
  );

/**
  Replacement for PcdGetPtr that allows the boot parameter filesystem
  to override the returned string.

  @param ptr   A Pcd entry identifier (like PcdDefaultBootArgument).

**/
#define BPO_PcdGetPtr(ptr) _BPO_PcdGetPtr(#ptr, PcdGetPtr(ptr))

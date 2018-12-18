/** @file
 *
 * Support a file system fed by the BlueField boot stream.
 *
 * Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.
 * Copyright (c) 2015, Mellanox Technologies Inc.  All rights reserved.
 *
 * This program and the accompanying materials are licensed and made
 * available under the terms and conditions of the BSD License which
 * accompanies this distribution.  The full text of the license may be found
 * at http://opensource.org/licenses/bsd-license.php
 *
 * THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 * WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 */

#ifndef __BFB_FS_H__
#define __BFB_FS_H__

EFI_STATUS
VolumeOpen (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
  OUT EFI_FILE                        **Root
  );

EFI_STATUS
FileOpen (
  IN  EFI_FILE  *This,
  OUT EFI_FILE  **NewHandle,
  IN  CHAR16    *FileName,
  IN  UINT64    OpenMode,
  IN  UINT64    Attributes
  );

EFI_STATUS
FileClose (
  IN EFI_FILE  *This
  );

EFI_STATUS
FileDelete (
  IN EFI_FILE *This
  );

EFI_STATUS
FileRead (
  IN     EFI_FILE  *This,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  );

EFI_STATUS
FileWrite (
  IN     EFI_FILE *This,
  IN OUT UINTN    *BufferSize,
  IN     VOID     *Buffer
  );

EFI_STATUS
FileGetPosition (
  IN  EFI_FILE    *File,
  OUT UINT64      *Position
  );

EFI_STATUS
FileSetPosition (
  IN EFI_FILE *File,
  IN UINT64   Position
  );

EFI_STATUS
FileGetInfo (
  IN     EFI_FILE  *This,
  IN     EFI_GUID  *InformationType,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  );

EFI_STATUS
FileSetInfo (
  IN EFI_FILE  *This,
  IN EFI_GUID  *InformationType,
  IN UINTN     BufferSize,
  IN VOID      *Buffer
  );

EFI_STATUS
FileFlush (
  IN EFI_FILE *File
  );

#endif // __BFB_FS_H__


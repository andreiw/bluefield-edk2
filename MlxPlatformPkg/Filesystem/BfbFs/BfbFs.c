/** @file
 *
 * Support a file system fed by the BlueField boot stream.
 *
 * Copyright (c) 2008 - 2009, Apple Inc.  All rights reserved.
 * Portions copyright (c) 2011 - 2014, ARM Ltd.  All rights reserved.
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

#include <Uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/SimpleFileSystem.h>

#include "BfbFs.h"

#include "rsh_def.h"
#include "bluefield_boot.h"

// Note: all of the code that processes the boot stream assumes that we are
// running in little-endian mode, and will need minor tweaks if that is
// ever not true.

// Last header read from boot data stream
STATIC union boot_image_header mHeader;

// Have we read a header from the data stream yet?
STATIC UINT8 mHeaderValid;

// What's the partial CRC of the data read so far?
STATIC UINT32 mPartialCrc;

// Have we seen a fatal error in the input stream?
STATIC INTN mFatalError;

// How many bytes have been read from the stream but not used?  Note that
// this may include padding bytes at the end of an image which are not
// counted in the image's length.
STATIC INTN mResidueBytes;

// What are those bytes?  They're the low-order bytes of this word,
// starting with the lowest-order.
STATIC UINT64 mResidue;

// How many bytes have we read or skipped from the current image?
STATIC INTN mCurOffset;


/** Read bytes from the current image into a buffer, or throw them away if
 *  the buffer is NULL.  We also keep track of the CRC of the data seen so
 *  that we can check it at the end of the file.
 *
 * @param FileId Numeric ID of the file being read.
 * @param Buffer Destination for bytes read.
 * @param BufferSize Space available in Buffer.
 * @return Nonzero on error, else zero.
 */
INTN
BfbRead (
  IN     UINTN FileId,
  IN OUT VOID  *Buffer,
  IN     UINTN BufferSize
  )
{
  if (mFatalError || !mHeaderValid || mHeader.data.image_id != FileId) {
    return 1;
  }

  UINT8 *Buffer8 = (UINT8 *)Buffer;

  mCurOffset += BufferSize;

  while (BufferSize > 0 && mResidueBytes > 0) {
    BufferSize--;
    mResidueBytes--;
    if (Buffer8 != NULL) {
      *Buffer8++ = mResidue & 0xFF;
    }
    mResidue >>= 8;
  }

  UINT64 *Buffer64 = (UINT64 *)Buffer8;
  UINT64 Count = 0;

  for (; BufferSize >= 8; BufferSize -= 8) {
    if (Count <= 0) {
      while ((Count = MmioRead64 (PcdGet64 (PcdRshimBase) +
                                  RSH_BOOT_FIFO_COUNT)) <= 0)
        ;
    }

    UINT64 Data = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_DATA);

    // FIXME use a compiler instrinsic here, once we have one
    __asm__ ("crc32x %w0, %w0, %x1" : "+r" (mPartialCrc) : "r" (Data));
    if (Buffer64 != NULL) {
      *Buffer64++ = Data;
    }
    Count--;
  }

  if (BufferSize > 0) {
    Buffer8 = (UINT8 *)Buffer64;

    if (Count <= 0) {
      while ((Count = MmioRead64 (PcdGet64 (PcdRshimBase) +
                                  RSH_BOOT_FIFO_COUNT)) <= 0)
        ;
    }

    UINT64 Data = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_DATA);

    // FIXME use a compiler instrinsic here, once we have one
    __asm__ ("crc32x %w0, %w0, %x1" : "+r" (mPartialCrc) : "r" (Data));
    INTN i;
    for (i = 0; i < BufferSize; i++) {
      if (Buffer8 != NULL) {
        *Buffer8++ = Data & 0xFF;
      }
      Data >>= 8;
    }

    mResidue = Data;
    mResidueBytes = 8 - BufferSize;
  }

  // If we've consumed the entire image, then the CRC should match
  // what was in the header.

  if (mCurOffset >= mHeader.data.image_len &&
      mHeader.data.image_crc != ~mPartialCrc) {
    DEBUG ((EFI_D_ERROR,
      "BlueField boot: image %d bad CRC: expected 0x%x actual 0x%x\n",
      mHeader.data.image_id, mHeader.data.image_crc, ~mPartialCrc));
    return 1;
  }

  return 0;
}


/** Read the next header from the data stream, skipping any unread bytes in
 *  the current image first.
 *
 * @return Nonzero on error, else zero.
 */
STATIC UINTN
BfbGetNextHeader (
  VOID
  )
{
  if (mFatalError) {
    return 1;
  }

  if (mHeaderValid) {
    BfbRead (mHeader.data.image_id, NULL, mHeader.data.image_len - mCurOffset);
  }

  while (MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_COUNT) < 3)
    ;

  mHeader.words[0] = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_DATA);
  mHeader.words[1] = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_DATA);
  mHeader.words[2] = MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_DATA);

  if (mHeader.data.magic != BFB_IMGHDR_MAGIC) {
    DEBUG ((EFI_D_ERROR, "BlueField boot: bad magic number 0x%x\n",
           mHeader.data.magic));
    mFatalError = 1;
    return 1;
  }

  if (mHeader.data.major != BFB_IMGHDR_MAJOR) {
    DEBUG ((EFI_D_ERROR, "BlueField boot: bad major version %d\n",
          mHeader.data.major));
    mFatalError = 1;
    return 1;
  }

  // Discard any extra header words.
  if (mHeader.data.hdr_len > 3)
  {
    INTN i;
    for (i = 0; i < mHeader.data.hdr_len - 3; i++)
      (void) MmioRead64 (PcdGet64 (PcdRshimBase) + RSH_BOOT_FIFO_DATA);
  }

  DEBUG ((EFI_D_INFO,
    "BlueField ImgHdr V%d.%d Len %d ID %d ImLen %d HdCRC 0x%x FolIm 0x%lx\n",
    mHeader.data.major, mHeader.data.minor, mHeader.data.hdr_len,
    mHeader.data.image_id, mHeader.data.image_len, mHeader.data.image_crc,
    mHeader.data.following_images));

  mPartialCrc = ~0;
  mResidueBytes = 0;
  mCurOffset = 0;
  mHeaderValid = 1;

  return 0;
}

/** Translate a filename into an image ID.
 * @param SearchFileName Filename to look up.
 * @return The image ID, or -1 if the filename could not be translated.
 */
STATIC INTN
BfbFileToId (
  IN CHAR16* SearchFileName
  )
{
  struct FileToId {
    CONST CHAR16 *FileName;  // Input name.
    UINTN Id;                // Output file ID.
  };

  // Note that if you change any of the ID values in this table, you'll also
  // need to change a similar table in the mkbfb tool, which generates
  // BlueField boot streams.  If you change the BootXXX filenames, you will
  // need to change the names in
  // MlnxPlatformPkg/Library/PlatformBdsLib/DefaultBootOption.c,
  // and if you change the other filenames, you'll need to change the command
  // line passed to the Linux kernel.
  STATIC CONST struct FileToId FileToId[] = {
    { L"initramfs", 63 },
    { L"Image", 62 },
    { L"UefiTests", 61 },
    { L"BootTimeOut", 60 },
    { L"BootArgument", 59 },
    { L"BootDevicePath", 58 },
    { L"BootDescription", 57 },
    { L"BootDtb", 56 },
    { L"BootAcpi", 55 },
    { NULL, 0 },
  };

  CONST struct FileToId *Ptr;

  for (Ptr = FileToId; Ptr->FileName != NULL; Ptr++) {
    if (!StrCmp (SearchFileName, Ptr->FileName)) {
      return Ptr->Id;
    }
  }

  return -1;
}


/** Consume data from the boot stream until a specified file is at its
 *  head.
 * @param FileName Name of the file to look for.
 * @param FileId Returned file ID, to be passed to other functions.
 * @retval EFI_SUCCESS      The file was found.
 * @retval EFI_NOT_FOUND    The file is not present in the boot stream.
 * @retval EFI_DEVICE_ERROR The boot stream is misformatted or could not be
 *                          read.
 */
STATIC EFI_STATUS
BfbOpen (
  IN     CHAR16* FileName,
     OUT UINTN   *FileId)
{
  INTN Id = BfbFileToId (FileName);

  if (Id < 0) {
    return EFI_NOT_FOUND;
  }

  if (!mHeaderValid && BfbGetNextHeader ()) {
    return EFI_DEVICE_ERROR;
  }

  if (mHeader.data.image_id != Id &&
      (mHeader.data.following_images & (1ULL << Id)) == 0) {
    return EFI_NOT_FOUND;
  }

  while (mHeader.data.image_id != Id) {
    if (BfbGetNextHeader ()) {
      return EFI_DEVICE_ERROR;
    }
  }

  *FileId = Id;

  return EFI_SUCCESS;
}

/** Provide the length of the image at the head of the boot stream.
 * @param FileId File ID, which must match that of the current image.
 * @param FileLen Returned file length.
 * @return Nonzero on error, else 0.
 */
STATIC INTN
BfbGetLen (
  IN     UINTN  FileId,
  IN OUT UINT64 *FileLen
  )
{
  if (!mHeaderValid || mHeader.data.image_id != FileId) {
    return 1;
  }

  *FileLen = mHeader.data.image_len;
  return 0;
}

#define DEFAULT_BFB_FS_LABEL   L"BfbFs"

STATIC CHAR16 *mBfbFsLabel;

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL gBfbFs = {
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION,
  VolumeOpen
};

EFI_FILE gBfbFsFile = {
  EFI_FILE_PROTOCOL_REVISION,
  FileOpen,
  FileClose,
  FileDelete,
  FileRead,
  FileWrite,
  FileGetPosition,
  FileSetPosition,
  FileGetInfo,
  FileSetInfo,
  FileFlush
};

//
// Device path for Bfb. It contains our autogened Caller ID GUID.
//
typedef struct {
  VENDOR_DEVICE_PATH        Guid;
  EFI_DEVICE_PATH_PROTOCOL  End;
} BFB_DEVICE_PATH;

BFB_DEVICE_PATH gDevicePath = {
  {
    { HARDWARE_DEVICE_PATH, HW_VENDOR_DP,
      { sizeof (VENDOR_DEVICE_PATH), 0 }
    },
    EFI_CALLER_ID_GUID
  },
  { END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE,
    { sizeof (EFI_DEVICE_PATH_PROTOCOL), 0 }
  }
};

typedef struct {
  LIST_ENTRY    Link;
  UINT64        Signature;
  EFI_FILE      File;
  CHAR16        *FileName;
  UINT64        OpenMode;
  UINT32        Position;
  UINTN         FileId;
  BOOLEAN       IsRoot;
  EFI_FILE_INFO Info;
} BFB_FCB;

#define BFB_FCB_SIGNATURE      SIGNATURE_32( 'B', 'F', 'F', 'C' )
#define BFB_FCB_FROM_THIS(a)   CR(a, BFB_FCB, File, BFB_FCB_SIGNATURE)
#define BFB_FCB_FROM_LINK(a)   CR(a, BFB_FCB, Link, BFB_FCB_SIGNATURE);

EFI_HANDLE  gInstallHandle = NULL;
LIST_ENTRY  gFileList = INITIALIZE_LIST_HEAD_VARIABLE (gFileList);

STATIC
BFB_FCB *
AllocateFCB (
  VOID
  )
{
  BFB_FCB *Fcb = AllocateZeroPool (sizeof (BFB_FCB));

  if (Fcb != NULL) {
    CopyMem (&Fcb->File, &gBfbFsFile, sizeof (gBfbFsFile));
    Fcb->Signature = BFB_FCB_SIGNATURE;
  }

  return Fcb;
}

STATIC
VOID
FreeFCB (
  IN BFB_FCB *Fcb
  )
{
  // Remove Fcb from gFileList.
  RemoveEntryList (&Fcb->Link);

  // To help debugging...
  Fcb->Signature = 0;

  FreePool (Fcb);
}



EFI_STATUS
VolumeOpen (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
  OUT EFI_FILE                        **Root
  )
{
  BFB_FCB *RootFcb = NULL;

  if (Root == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  RootFcb = AllocateFCB ();
  if (RootFcb == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  RootFcb->IsRoot = TRUE;
  RootFcb->Info.Attribute = EFI_FILE_READ_ONLY | EFI_FILE_DIRECTORY;

  InsertTailList (&gFileList, &RootFcb->Link);

  *Root = &RootFcb->File;

  return EFI_SUCCESS;
}

/**
  Open a file from the boot stream.

  @param[in]   This        A pointer to the EFI_FILE_PROTOCOL instance that is
                           the file handle to source location.
  @param[out]  NewHandle   A pointer to the location to return the opened
                           handle for the new file.
  @param[in]   FileName    The Null-terminated string of the name of the file
                           to be opened.
  @param[in]   OpenMode    The mode to open the file: only Read is allowed.
  @param[in] Attributes    Unused.

  @retval  EFI_SUCCESS            The file was open.
  @retval  EFI_NOT_FOUND          The specified file could not be found.
  @retval  EFI_DEVICE_ERROR       The boot stream is corrupt.
  @retval  EFI_WRITE_PROTECTED    Attempt to open a file for writing.
  @retval  EFI_OUT_OF_RESOURCES   Not enough resources were available to open
                                  the file.
  @retval  EFI_INVALID_PARAMETER  At least one of the parameters is invalid.

**/
EFI_STATUS
FileOpen (
  IN  EFI_FILE  *This,
  OUT EFI_FILE  **NewHandle,
  IN  CHAR16    *FileName,
  IN  UINT64    OpenMode,
  IN  UINT64    Attributes
  )
{
  BFB_FCB        *FileFcb;

  if ((FileName == NULL) || (NewHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (OpenMode & (EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE)) {
    return EFI_WRITE_PROTECTED;
  }

  if (OpenMode != EFI_FILE_MODE_READ) {
    return EFI_INVALID_PARAMETER;
  }

  // Opening '/', '\', '.', or the NULL pathname is trying to open the root
  // directory
  if ((StrCmp (FileName, L"\\") == 0) ||
      (StrCmp (FileName, L"/")  == 0) ||
      (StrCmp (FileName, L"")   == 0) ||
      (StrCmp (FileName, L".")  == 0)    ) {
    return (VolumeOpen (&gBfbFs, NewHandle));
  }

  UINTN Id;
  EFI_STATUS Result = BfbOpen(FileName, &Id);
  if (Result != EFI_SUCCESS) {
    return Result;
  }

  // Allocate a control block and fill it
  FileFcb = AllocateFCB ();
  if (FileFcb == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  UINT64 Size;
  if (BfbGetLen (Id, &Size)) {
    return EFI_DEVICE_ERROR;
  }

  FileFcb->FileName  = AllocateCopyPool (StrSize (FileName), FileName);
  FileFcb->FileId    = Id;
  FileFcb->Position  = 0;
  FileFcb->IsRoot    = 0;
  FileFcb->OpenMode  = OpenMode;

  FileFcb->Info.FileSize     = Size;
  FileFcb->Info.PhysicalSize = Size;
  FileFcb->Info.Attribute    = EFI_FILE_READ_ONLY;

  InsertTailList (&gFileList, &FileFcb->Link);

  *NewHandle = &FileFcb->File;

  return EFI_SUCCESS;
}

/**
  Close a specified file handle.

  @param[in]  This  A pointer to the EFI_FILE_PROTOCOL instance that is the
                    file handle to close.

  @retval  EFI_SUCCESS            The file was closed.
  @retval  EFI_INVALID_PARAMETER  The parameter "This" is NULL.

**/
EFI_STATUS
FileClose (
  IN EFI_FILE  *This
  )
{
  BFB_FCB   *Fcb;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Fcb = BFB_FCB_FROM_THIS (This);

  if (!Fcb->IsRoot) {
    FreePool (Fcb->FileName);
  }

  FreeFCB (Fcb);

  return EFI_SUCCESS;
}

/**
  Close and delete a file.

  @param[in]  This  A pointer to the EFI_FILE_PROTOCOL instance that is the
                    file handle to delete.

  @retval  EFI_SUCCESS              The file was closed and deleted.
  @retval  EFI_WARN_DELETE_FAILURE  The handle was closed, but the file was
                                    not deleted.
  @retval  EFI_INVALID_PARAMETER    The parameter "This" is NULL.

**/
EFI_STATUS
FileDelete (
  IN EFI_FILE *This
  )
{
  BFB_FCB        *Fcb;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Fcb = BFB_FCB_FROM_THIS (This);

  if (!Fcb->IsRoot) {
    // Close the file if it's open.  Disregard return status,
    // since it might give an error if the file isn't open.
    This->Close (This);
  }
  return EFI_WARN_DELETE_FAILURE;
}

/**
  Read data from an open file.

  @param[in]      This        A pointer to the EFI_FILE_PROTOCOL instance that
                              is the file handle to read data from.
  @param[in out]  BufferSize  On input, the size of the Buffer. On output, the
                              amount of data returned in Buffer. In both cases,
                              the size is measured in bytes.
  @param[out]     Buffer      The buffer into which the data is read.

  @retval  EFI_SUCCESS            The data was read.
  @retval  EFI_DEVICE_ERROR       On entry, the current file position is
                                  beyond the end of the file, or the boot
                                  stream is corrupt.
  @retval  EFI_INVALID_PARAMETER  At least one of the three input pointers is
                                  NULL.

**/
EFI_STATUS
FileRead (
  IN     EFI_FILE  *This,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  )
{
  BFB_FCB             *Fcb;

  if ((This == NULL) || (BufferSize == NULL) || (Buffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Fcb = BFB_FCB_FROM_THIS (This);

  // We could theoretically figure out what files are left in the stream by
  // looking at the image header bitmap, so that we could create a
  // directory listing, but it would be confusing for that to change as
  // we've read files, so we don't bother supporting directory reads.
  if (Fcb->IsRoot)
    return EFI_UNSUPPORTED;

  if (Fcb->Position > Fcb->Info.FileSize) {
    return EFI_DEVICE_ERROR;
  }

  if (Fcb->Position + *BufferSize > Fcb->Info.FileSize) {
    *BufferSize = Fcb->Info.FileSize - Fcb->Position;
  }

  if (BfbRead (Fcb->FileId, Buffer, *BufferSize)) {
    return EFI_DEVICE_ERROR;
  }

  Fcb->Position += *BufferSize;

  return EFI_SUCCESS;
}

/**
  Write data to an open file.

  @param[in]      This        A pointer to the EFI_FILE_PROTOCOL instance that
                              is the file handle to write data to.
  @param[in out]  BufferSize  On input, the size of the Buffer. On output, the
                              size of the data actually written. In both cases,
                              the size is measured in bytes.
  @param[in]      Buffer      The buffer of data to write.

  @retval  EFI_WRITE_PROTECTED   The data was not written.
  @retval  EFI_INVALID_PARAMETER At least one of the three input pointers is
                                  NULL.

**/
EFI_STATUS
FileWrite (
  IN     EFI_FILE *This,
  IN OUT UINTN    *BufferSize,
  IN     VOID     *Buffer
  )
{
  if ((This == NULL) || (BufferSize == NULL) || (Buffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_WRITE_PROTECTED;
}

/**
  Return a file's current position.

  @param[in]   This      A pointer to the EFI_FILE_PROTOCOL instance that is
                         the file handle to get the current position on.
  @param[out]  Position  The address to return the file's current position
                         value.

  @retval  EFI_SUCCESS            The position was returned.
  @retval  EFI_INVALID_PARAMETER  The parameter "This" or "Position" is NULL.

**/
EFI_STATUS
FileGetPosition (
  IN  EFI_FILE    *This,
  OUT UINT64      *Position
  )
{
  BFB_FCB *Fcb;

  if ((This == NULL) || (Position == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Fcb = BFB_FCB_FROM_THIS (This);

  *Position = Fcb->Position;

  return EFI_SUCCESS;
}

/**
  Set a file's current position.

  @param[in]  This      A pointer to the EFI_FILE_PROTOCOL instance that is
                        the file handle to set the requested position on.
  @param[in]  Position  The byte position from the start of the file to set.

  @retval  EFI_SUCCESS       The position was set.
  @retval  EFI_DEVICE_ERROR  The semi-hosting positionning operation failed.
  @retval  EFI_UNSUPPORTED   The seek request for nonzero is not valid on open
                             directories.
  @retval  EFI_INVALID_PARAMETER  The parameter "This" is NULL.

**/
EFI_STATUS
FileSetPosition (
  IN EFI_FILE *This,
  IN UINT64   Position
  )
{
  BFB_FCB        *Fcb;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Fcb = BFB_FCB_FROM_THIS (This);

  if (Fcb->IsRoot) {
    if (Position != 0) {
      return EFI_UNSUPPORTED;
    }
  } else {
    //
    // UEFI Spec section 12.5:
    // "Seeking to position 0xFFFFFFFFFFFFFFFF causes the current position to
    // be set to the end of the file."
    //
    if (Position == 0xFFFFFFFFFFFFFFFF) {
      Position = Fcb->Info.FileSize;
    }

    // Can't seek backwards.
    if (Position < Fcb->Position) {
      return EFI_DEVICE_ERROR;
    }

    if (BfbRead (Fcb->FileId, NULL, Position - Fcb->Position)) {
      return EFI_DEVICE_ERROR;
    }
  }

  Fcb->Position = Position;

  return EFI_SUCCESS;
}

/**
  Return information about a file.

  @param[in]      Fcb         A pointer to the description of an open file.
  @param[in out]  BufferSize  The size, in bytes, of Buffer.
  @param[out]     Buffer      A pointer to the data buffer to return. Not NULL
                              if "*BufferSize" is greater than 0.

  @retval  EFI_SUCCESS            The information was returned.
  @retval  EFI_BUFFER_TOO_SMALL   The BufferSize is too small to return the
                                  information.  BufferSize has been updated
                                  with the size needed to complete the request.
**/
STATIC
EFI_STATUS
GetFileInfo (
  IN     BFB_FCB       *Fcb,
  IN OUT UINTN         *BufferSize,
  OUT    VOID          *Buffer
  )
{
  EFI_FILE_INFO   *Info = NULL;
  UINTN           NameSize;
  UINTN           ResultSize;

  if (Fcb->IsRoot == TRUE) {
    NameSize   = 0;
    ResultSize = SIZE_OF_EFI_FILE_INFO + sizeof (CHAR16);
  } else {
    NameSize   = StrSize (Fcb->FileName);
    ResultSize = SIZE_OF_EFI_FILE_INFO + NameSize;
  }

  if (*BufferSize < ResultSize) {
    *BufferSize = ResultSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  Info = Buffer;

  // Copy the current file info
  CopyMem (Info, &Fcb->Info, SIZE_OF_EFI_FILE_INFO);

  // Fill in the structure
  Info->Size = ResultSize;

  if (Fcb->IsRoot == TRUE) {
    Info->FileName[0]  = L'\0';
  } else {
    StrCpy (Info->FileName, Fcb->FileName);
  }

  *BufferSize = ResultSize;

  return EFI_SUCCESS;
}

/**
  Return information about a file system.

  @param[in]      Fcb         A pointer to the description of an open file
                              which belongs to the file system, the information
                              is requested for.
  @param[in out]  BufferSize  The size, in bytes, of Buffer.
  @param[out]     Buffer      A pointer to the data buffer to return. Not NULL
                              if "*BufferSize" is greater than 0.

  @retval  EFI_SUCCESS            The information was returned.
  @retval  EFI_BUFFER_TOO_SMALL   The BufferSize is too small to return the
                                  information.  BufferSize has been updated
                                  with the size needed to complete the request.

**/
STATIC
EFI_STATUS
GetFilesystemInfo (
  IN     BFB_FCB      *Fcb,
  IN OUT UINTN        *BufferSize,
  OUT    VOID         *Buffer
  )
{
  EFI_FILE_SYSTEM_INFO  *Info;
  EFI_STATUS            Status;
  UINTN                 ResultSize;

  ResultSize = SIZE_OF_EFI_FILE_SYSTEM_INFO + StrSize (mBfbFsLabel);

  if (*BufferSize >= ResultSize) {
    ZeroMem (Buffer, ResultSize);
    Status = EFI_SUCCESS;

    Info = Buffer;

    Info->Size       = ResultSize;
    Info->ReadOnly   = TRUE;
    Info->VolumeSize = 0;
    Info->FreeSpace  = 0;
    Info->BlockSize  = 0;

    StrCpy (Info->VolumeLabel, mBfbFsLabel);
  } else {
    Status = EFI_BUFFER_TOO_SMALL;
  }

  *BufferSize = ResultSize;
  return Status;
}

/**
  Return information about a file or a file system.

  @param[in]      This             A pointer to the EFI_FILE_PROTOCOL instance
                                   that is the file handle the requested
                                   information is for.
  @param[in]      InformationType  The type identifier for the information
                                   being requested : EFI_FILE_INFO_ID or
                                   EFI_FILE_SYSTEM_INFO_ID or
                                   EFI_FILE_SYSTEM_VOLUME_LABEL_ID
  @param[in out]  BufferSize       The size, in bytes, of Buffer.
  @param[out]     Buffer           A pointer to the data buffer to return. The
                                   type of the data inside the buffer is
                                   indicated by InformationType.

  @retval  EFI_SUCCESS           The information was returned.
  @retval  EFI_UNSUPPORTED       The InformationType is not known.
  @retval  EFI_BUFFER_TOO_SMALL  The BufferSize is too small to return the
                                 information.  BufferSize has been updated with
                                 the size needed to complete the request.
  @retval  EFI_INVALID_PARAMETER  The parameter "This" or "InformationType" or
                                  "BufferSize" is NULL or "Buffer" is NULL and
                                  "*Buffersize" is greater than 0.
**/
EFI_STATUS
FileGetInfo (
  IN     EFI_FILE  *This,
  IN     EFI_GUID  *InformationType,
  IN OUT UINTN     *BufferSize,
  OUT    VOID      *Buffer
  )
{
  BFB_FCB      *Fcb;
  EFI_STATUS   Status;
  UINTN        ResultSize;

  if ((This == NULL)                         ||
      (InformationType == NULL)              ||
      (BufferSize == NULL)                   ||
      ((Buffer == NULL) && (*BufferSize > 0))  ) {
    return EFI_INVALID_PARAMETER;
  }

  Fcb = BFB_FCB_FROM_THIS (This);

  if (CompareGuid (InformationType, &gEfiFileSystemInfoGuid)) {
    Status = GetFilesystemInfo (Fcb, BufferSize, Buffer);
  } else if (CompareGuid (InformationType, &gEfiFileInfoGuid)) {
    Status = GetFileInfo (Fcb, BufferSize, Buffer);
  } else if (CompareGuid (InformationType,
                          &gEfiFileSystemVolumeLabelInfoIdGuid)) {
    ResultSize = StrSize (mBfbFsLabel);

    if (*BufferSize >= ResultSize) {
      StrCpy (Buffer, mBfbFsLabel);
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_BUFFER_TOO_SMALL;
    }

    *BufferSize = ResultSize;
  } else {
    Status = EFI_UNSUPPORTED;
  }

  return Status;
}


/**
  Set information about a file or a file system.

  @param[in]  This             A pointer to the EFI_FILE_PROTOCOL instance that
                               is the file handle the information is for.
  @param[in]  InformationType  The type identifier for the information being
                               set: EFI_FILE_INFO_ID or EFI_FILE_SYSTEM_INFO_ID
                               or EFI_FILE_SYSTEM_VOLUME_LABEL_ID
  @param[in]  BufferSize       The size, in bytes, of Buffer.
  @param[in]  Buffer           A pointer to the data buffer to write. The type
                               of the data inside the buffer is indicated by
                               InformationType.

  @retval  EFI_WRITE_PROTECTED    An attempt is being made to modify a
                                  read-only attribute.
  @retval  EFI_INVALID_PARAMETER  At least one of the parameters is invalid.
**/
EFI_STATUS
FileSetInfo (
  IN EFI_FILE  *This,
  IN EFI_GUID  *InformationType,
  IN UINTN     BufferSize,
  IN VOID      *Buffer
  )
{
  if ((This == NULL) || (InformationType == NULL) || (Buffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_WRITE_PROTECTED;
}

EFI_STATUS
FileFlush (
  IN EFI_FILE *File
  )
{
  BFB_FCB *Fcb;

  Fcb = BFB_FCB_FROM_THIS (File);

  if (Fcb->IsRoot) {
    return EFI_SUCCESS;
  } else {
    if ((Fcb->Info.Attribute & EFI_FILE_READ_ONLY)
        || !(Fcb->OpenMode & EFI_FILE_MODE_WRITE)) {
      return EFI_ACCESS_DENIED;
    } else {
      return EFI_SUCCESS;
    }
  }
}

EFI_STATUS
BfbFsEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS    Status;

  Status = EFI_NOT_FOUND;

  mBfbFsLabel = AllocateCopyPool (StrSize (DEFAULT_BFB_FS_LABEL),
                  DEFAULT_BFB_FS_LABEL);
  if (mBfbFsLabel == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gInstallHandle,
                  &gEfiSimpleFileSystemProtocolGuid, &gBfbFs,
                  &gEfiDevicePathProtocolGuid,       &gDevicePath,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    FreePool (mBfbFsLabel);
  }

  return Status;
}

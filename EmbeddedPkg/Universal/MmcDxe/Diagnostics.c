/** @file
 *
 * freePool( Buffer );
 *
  Diagnostics Protocol implementation for the MMC DXE driver

  Copyright (c) 2011-2014, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <stdio.h>
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>

#include "Mmc.h"

#define DIAGNOSTIC_LOGBUFFER_MAXCHAR  1024
#define MAX_CHARBUF_LINE  120

CHAR16* mLogBuffer = NULL;
UINTN   mLogRemainChar = 0;

CHAR16* mLogPostBuffer = NULL;
UINTN   mLogPostRemainChar = 0;

UINTN
PostInitLog (
  UINTN MaxBufferChar
  )
{
  UINTN status = 0;

  mLogPostRemainChar = MaxBufferChar;
  mLogPostBuffer = AllocatePool ((UINTN)MaxBufferChar * sizeof (CHAR16));
  if( mLogPostBuffer == NULL )
  {
    status = 1;
    mLogPostRemainChar = 0;
  }
  return status;
}

UINTN
PostLog (
  CONST CHAR16* Str
  )
{
  UINTN len = StrLen (Str);
  if (len <= mLogPostRemainChar) {
    mLogPostRemainChar -= len;
    StrCpy (mLogPostBuffer, Str);
    mLogPostBuffer += len;
    return len;
  } else {
    return 0;
  }
}

/**
  Interpret keyboard input.

  @retval  EFI_ABORTED  Get an 'ESC' key inputed.
  @retval  EFI_SUCCESS  Get an 'Y' or 'y' inputed.
  @retval  EFI_NOT_FOUND Get an 'N' or 'n' inputed..

**/
EFI_STATUS
GetResponse (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_INPUT_KEY Key;

  while (TRUE) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (!EFI_ERROR (Status)) {
      if (Key.ScanCode == SCAN_ESC) {
        return EFI_ABORTED;
      }

      switch (Key.UnicodeChar) {
      case L'y':
      case L'Y':
        gST->ConOut->OutputString (gST->ConOut, L"Y\n");
        return EFI_SUCCESS;
      case L'n':
      case L'N':
        gST->ConOut->OutputString (gST->ConOut, L"N\n");
        return EFI_ABORTED;
      }

    }
    DEBUG((DEBUG_ERROR, "ERROR: Status=%r",Status));
  }
}

UINTN
DiagnosticInitLog (
  UINTN MaxBufferChar
  )
{
  UINTN status = 0;
  mLogRemainChar = MaxBufferChar;
  mLogBuffer = AllocatePool ((UINTN)MaxBufferChar * sizeof (CHAR16));
  if(mLogBuffer == NULL )
  {
    mLogRemainChar = 0;
    status = 1;
  }
  return status;
}

UINTN
DiagnosticLog (
  CONST CHAR16* Str
  )
{
  UINTN len = StrLen (Str);
  if (len <= mLogRemainChar) {
    mLogRemainChar -= len;
    StrCpy (mLogBuffer, Str);
    mLogBuffer += len;
    return len;
  } else {
    return 0;
  }
}

VOID
GenerateRandomBuffer (
  VOID* Buffer,
  UINTN BufferSize
  )
{
  UINT64  i;
  UINT64* Buffer64 = (UINT64*)Buffer;

  for (i = 0; i < (BufferSize >> 3); i++) {
    *Buffer64 = i | (~i << 32);
    Buffer64++;
  }
}

BOOLEAN
CompareBuffer (
  VOID  *BufferA,
  VOID  *BufferB,
  UINTN BufferSize
  )
{
  UINTN i;
  UINT64* BufferA64 = (UINT64*)BufferA;
  UINT64* BufferB64 = (UINT64*)BufferB;

  for (i = 0; i < (BufferSize >> 3); i++) {
    if (*BufferA64 != *BufferB64) {
      DEBUG ((EFI_D_ERROR, "CompareBuffer: Error at %i", i));
      DEBUG ((EFI_D_ERROR, "(0x%lX) != (0x%lX)\n", *BufferA64, *BufferB64));
      return FALSE;
    }
    BufferA64++;
    BufferB64++;
  }
  return TRUE;
}

EFI_STATUS
MmcReadWriteDataTest (
  MMC_HOST_INSTANCE *MmcHostInstance,
  EFI_LBA           Lba,
  UINTN             BufferSize
  )
{
  VOID                        *BackBuffer;
  VOID                        *WriteBuffer;
  VOID                        *ReadBuffer;
  EFI_STATUS                  Status;

  // Check if a Media is Present
  if (!MmcHostInstance->BlockIo.Media->MediaPresent) {
    DiagnosticLog (L"ERROR: No Media Present\n");
    return EFI_NO_MEDIA;
  }

  if (MmcHostInstance->State != MmcTransferState) {
    DiagnosticLog (L"ERROR: MMC Host is not ready for Transfer state\n");
    return EFI_NOT_READY;
  }

  BackBuffer = AllocatePool (BufferSize);
  WriteBuffer = AllocatePool (BufferSize);
  ReadBuffer = AllocatePool (BufferSize);

  // Read (and save) buffer at a specific location
  Status = MmcReadBlocks (&(MmcHostInstance->BlockIo), MmcHostInstance->BlockIo.Media->MediaId,Lba,BufferSize,BackBuffer);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail to Read Block (1)\n");
    return Status;
  }

  // Write buffer at the same location
  GenerateRandomBuffer (WriteBuffer,BufferSize);
  Status = MmcWriteBlocks (&(MmcHostInstance->BlockIo), MmcHostInstance->BlockIo.Media->MediaId,Lba,BufferSize,WriteBuffer);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail to Write Block (1)\n");
    return Status;
  }

  // Read the buffer at the same location
  Status = MmcReadBlocks (&(MmcHostInstance->BlockIo), MmcHostInstance->BlockIo.Media->MediaId,Lba,BufferSize,ReadBuffer);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail to Read Block (2)\n");
    return Status;
  }

  // Check that is conform
  if (!CompareBuffer (ReadBuffer,WriteBuffer,BufferSize)) {
    DiagnosticLog (L"ERROR: Fail to Read/Write Block (1)\n");
    return EFI_INVALID_PARAMETER;
  }

  // Restore content at the original location
  Status = MmcWriteBlocks (&(MmcHostInstance->BlockIo), MmcHostInstance->BlockIo.Media->MediaId,Lba,BufferSize,BackBuffer);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail to Write Block (2)\n");
    return Status;
  }

  // Read the restored content
  Status = MmcReadBlocks (&(MmcHostInstance->BlockIo), MmcHostInstance->BlockIo.Media->MediaId,Lba,BufferSize,ReadBuffer);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail to Read Block (3)\n");
    return Status;
  }

  // Check the content is correct
  if (!CompareBuffer (ReadBuffer,BackBuffer,BufferSize)) {
    DiagnosticLog (L"ERROR: Fail to Read/Write Block (2)\n");
    return EFI_INVALID_PARAMETER;
  }

  FreePool (ReadBuffer);
  FreePool (WriteBuffer);
  FreePool (BackBuffer);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MmcDriverDiagnosticsRunDiagnostics (
  IN  EFI_DRIVER_DIAGNOSTICS_PROTOCOL               *This,
  IN  EFI_HANDLE                                    ControllerHandle,
  IN  EFI_HANDLE                                    ChildHandle  OPTIONAL,
  IN  EFI_DRIVER_DIAGNOSTIC_TYPE                    DiagnosticType,
  IN  CHAR8                                         *Language,
  OUT EFI_GUID                                      **ErrorType,
  OUT UINTN                                         *BufferSize,
  OUT CHAR16                                        **Buffer
  )
{
  LIST_ENTRY              *CurrentLink;
  MMC_HOST_INSTANCE       *MmcHostInstance;
  EFI_STATUS              Status = EFI_SUCCESS;

  if ((Language         == NULL) ||
      (ErrorType        == NULL) ||
      (Buffer           == NULL) ||
      (ControllerHandle == NULL) ||
      (BufferSize       == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  // Check Language is supported (i.e. is "en-*" - only English is supported)
  if (AsciiStrnCmp (Language, "en", 2) != 0) {
    return EFI_UNSUPPORTED;
  }

    Status = gST->ConOut->OutputString( gST->ConOut,
                 L"WARNING: Contents of MMC may get corrupted\n\r");

    Status = gST->ConOut->OutputString( gST->ConOut,
                L"Excessive writes to MMC will decrease its shelf life.\n\r");

    Status = gST->ConOut->OutputString( gST->ConOut,
                L"Run MMC Diagnostics? (Y/N) \n\r");

  Status = GetResponse();
  if( Status !=  EFI_SUCCESS )
  {
    return Status;
  }

  Status = EFI_SUCCESS;
  *ErrorType  = NULL;
  *BufferSize = DIAGNOSTIC_LOGBUFFER_MAXCHAR;
  if( DiagnosticInitLog (DIAGNOSTIC_LOGBUFFER_MAXCHAR) == 1 )
   {
        DEBUG ((EFI_D_ERROR, "MMC Driver Diagnostics: Unable to allocate buffer!"));
        return EFI_ABORTED;
   }
   else
   {
       DEBUG ((EFI_D_ERROR, "MMC Driver Diagnostics: Running....."));
   }

  *Buffer = mLogBuffer;
  DiagnosticLog (L"MMC Driver Diagnostics");

  // Find the MMC Host instance on which we have been asked to run diagnostics
  MmcHostInstance = NULL;
  CurrentLink = mMmcHostPool.ForwardLink;
  while (CurrentLink != NULL && CurrentLink != &mMmcHostPool && (Status == EFI_SUCCESS)) {
    MmcHostInstance = MMC_HOST_INSTANCE_FROM_LINK(CurrentLink);
    ASSERT(MmcHostInstance != NULL);

    if (MmcHostInstance->MmcHandle == ControllerHandle) {
      break;
    }
    CurrentLink = CurrentLink->ForwardLink;
  }

  // If we didn't find the controller, don't return EFI_UNSUPPORTED
  // run anyway

  if ((MmcHostInstance == NULL))
  {
    return EFI_UNSUPPORTED;
  }




  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail on First Block\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  // LBA=2 Size=BlockSize
  DiagnosticLog (L"MMC Driver Diagnostics - Test: Second Block");
  Status = MmcReadWriteDataTest (MmcHostInstance, 2, MmcHostInstance->BlockIo.Media->BlockSize);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail on Second Block\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  // LBA=10 Size=BlockSize
  DiagnosticLog (L"MMC Driver Diagnostics - Test: Any Block");
  Status = MmcReadWriteDataTest (MmcHostInstance, MmcHostInstance->BlockIo.Media->LastBlock >> 1, MmcHostInstance->BlockIo.Media->BlockSize);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail on Any Block\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  // LBA=LastBlock Size=BlockSize
  DiagnosticLog (L"MMC Driver Diagnostics - Test: Last Block");
  Status = MmcReadWriteDataTest (MmcHostInstance, MmcHostInstance->BlockIo.Media->LastBlock, MmcHostInstance->BlockIo.Media->BlockSize);
  if (Status != EFI_SUCCESS) {
    DiagnosticLog (L"ERROR: Fail on Last Block\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  // LBA=1 Size=2*BlockSize
  DiagnosticLog (L"MMC Driver Diagnostics - Test: First Block / 2 BlockSSize");
  Status = MmcReadWriteDataTest (MmcHostInstance, 1, 2 * MmcHostInstance->BlockIo.Media->BlockSize);

  if (Status != EFI_SUCCESS)
  {
    DiagnosticLog (L"ERROR: Fail on 2 Blocks\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  if (Status != EFI_SUCCESS)
  {
        DEBUG ((EFI_D_ERROR, "FAILED! %s\n", *Buffer ));
        DEBUG((DEBUG_ERROR, "Status=%r",Status));
  }
  else
  {
      DEBUG ((EFI_D_ERROR, "Passed.\n"));
  }

  return Status;
}

//
// EFI Driver Diagnostics 2 Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_DRIVER_DIAGNOSTICS2_PROTOCOL gMmcDriverDiagnostics2 = {
  (EFI_DRIVER_DIAGNOSTICS2_RUN_DIAGNOSTICS) MmcDriverDiagnosticsRunDiagnostics,
  "en"
};


EFI_STATUS
MmcReadDataPostTest (
  MMC_HOST_INSTANCE *MmcHostInstance,
  EFI_LBA           Lba,
  UINTN             BufferSize
  )
{
  VOID                        *BackBuffer;
  EFI_STATUS                  Status;

  // Check if a Media is Present
  if (!(Status = MmcHostInstance->BlockIo.Media->MediaPresent)) {
    PostLog (L"ERROR: No Media Present\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return EFI_NO_MEDIA;
  }

  if (MmcHostInstance->State != MmcTransferState) {
    PostLog (L"ERROR: MMC is not ready for Transfer state\n");
    return EFI_NOT_READY;
  }

  BackBuffer = AllocatePool (BufferSize);

  // Read (and save) buffer at a specific location
  Status = MmcReadBlocks (&(MmcHostInstance->BlockIo), MmcHostInstance->BlockIo.Media->MediaId,Lba,BufferSize,BackBuffer);
  if (Status != EFI_SUCCESS) {
    PostLog (L"ERROR: Fail to Read Block (1)\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  FreePool (BackBuffer);

  return Status;
}

EFI_STATUS
EFIAPI
MmcDriverDiagnosticsRunPost (
  IN  EFI_HANDLE                                    ControllerHandle,
  IN  EFI_HANDLE                                    ChildHandle  OPTIONAL
  )
{
  LIST_ENTRY              *CurrentLink;
  MMC_HOST_INSTANCE       *MmcHostInstance;
  EFI_STATUS              Status;
  CHAR16*                 Buffer = NULL;

  Status = EFI_SUCCESS;
   if( PostInitLog ((UINTN)DIAGNOSTIC_LOGBUFFER_MAXCHAR) != 0 )
   {
        DEBUG ((EFI_D_ERROR, "MMC Driver POST: Unable to allocate buffer!"));
        return EFI_ABORTED;
   }
   else
   {
       DEBUG ((EFI_D_ERROR, "MMC Driver POST: Running....."));
   }
  Buffer = mLogPostBuffer;
  PostLog (L"MMC Driver POST");

  // Find the MMC Host instance on which we have been asked to run diagnostics
  MmcHostInstance = NULL;
  CurrentLink = mMmcHostPool.ForwardLink;
  while (CurrentLink != NULL && CurrentLink != &mMmcHostPool && (Status == EFI_SUCCESS)) {
    MmcHostInstance = MMC_HOST_INSTANCE_FROM_LINK(CurrentLink);
    ASSERT(MmcHostInstance != NULL);

    if (MmcHostInstance->MmcHandle == ControllerHandle) {
      break;
    }
    CurrentLink = CurrentLink->ForwardLink;
  }

  // If we didn't find the controller, dont' return EFI_UNSUPPORTED
  // run anyway

  if ((MmcHostInstance == NULL)) {
   //   || (MmcHostInstance->MmcHandle != ControllerHandle))
   return EFI_UNSUPPORTED;
  }

  // LBA=1 Size=BlockSize
  PostLog (L"MMC Driver POST: First Block");
  Status = MmcReadDataPostTest (MmcHostInstance, 1, MmcHostInstance->BlockIo.Media->BlockSize);
  if (Status != EFI_SUCCESS) {
    PostLog (L"ERROR: Fail on First Block\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }


  // LBA=LastBlock Size=BlockSize
  PostLog (L"MMC Driver POST: Last Block");
  Status = MmcReadDataPostTest (MmcHostInstance, MmcHostInstance->BlockIo.Media->LastBlock, MmcHostInstance->BlockIo.Media->BlockSize);
  if (Status != EFI_SUCCESS) {
    PostLog (L"ERROR: Fail on Last Block\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  // LBA=1 Size=2*BlockSize
  PostLog (L"MMC Driver POST: First Block / 2 BlockSSize");
  Status = MmcReadDataPostTest (MmcHostInstance, 1, 2 * MmcHostInstance->BlockIo.Media->BlockSize);
  if (Status != EFI_SUCCESS) {
    PostLog (L"ERROR: Fail on 2 Blocks\n");
    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    return Status;
  }

  if (Status != EFI_SUCCESS)
  {

    DEBUG((DEBUG_ERROR, "Status=%r",Status));
    if( *Buffer )
    {
      DEBUG ((EFI_D_ERROR, "%s\n", Buffer ));
    }
    DEBUG ((EFI_D_ERROR, "MMC Diagnostic FAILED!\n"));
  }
  else
  {
    DEBUG ((EFI_D_ERROR, "MMC Diagnostic Passed.\n"));
  }

  FreePool (Buffer);

  return Status;
}


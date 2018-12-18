/** @file
  Provides variable driver extended services.

Copyright (c) 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Variable.h"

/**
  Finds variable in storage blocks of volatile and non-volatile storage areas.

  This code finds variable in storage blocks of volatile and non-volatile storage areas.
  If VariableName is an empty string, then we just return the first
  qualified variable without comparing VariableName and VendorGuid.

  @param[in]  VariableName          Name of the variable to be found.
  @param[in]  VendorGuid            Variable vendor GUID to be found.
  @param[out] VariableInfo          Pointer to AUTH_VARIABLE_INFO structure for
                                    output of the variable found.

  @retval EFI_INVALID_PARAMETER     If VariableName is not an empty string,
                                    while VendorGuid is NULL.
  @retval EFI_SUCCESS               Variable successfully found.
  @retval EFI_NOT_FOUND             Variable not found

**/
EFI_STATUS
EFIAPI
VariableExLibFindVariable (
  IN  CHAR16                *VariableName,
  IN  EFI_GUID              *VendorGuid,
  OUT AUTH_VARIABLE_INFO    *VariableInfo
  )
{
  EFI_STATUS                    Status;
  VARIABLE_POINTER_TRACK        Variable;
  AUTH_VAR_HEADER              *Header;

  Status = FindVariable (
             VariableName,
             VendorGuid,
             &Variable,
             &mVariableModuleGlobal->VariableGlobal
             );
  if (EFI_ERROR (Status)) {
    VariableInfo->Data = NULL;
    VariableInfo->DataSize = 0;
    VariableInfo->Attributes = 0;
    VariableInfo->PubKeyIndex = 0;
    VariableInfo->MonotonicCount = 0;
    VariableInfo->TimeStamp = NULL;
    return Status;
  }

  Header = Variable.CurrPtr;

  VariableInfo->DataSize        = Header->DataSize;
  VariableInfo->Data            = GetVariableDataPtr (Header);
  VariableInfo->Attributes      = Header->Attributes;
  VariableInfo->PubKeyIndex     = Header->PubKeyIndex;
  VariableInfo->MonotonicCount  = ReadUnaligned64 (&(Header->MonotonicCount));
  VariableInfo->TimeStamp       = &Header->TimeStamp;

  return EFI_SUCCESS;
}

/**
  Finds next variable in storage blocks of volatile and non-volatile storage areas.

  This code finds next variable in storage blocks of volatile and non-volatile storage areas.
  If VariableName is an empty string, then we just return the first
  qualified variable without comparing VariableName and VendorGuid.

  @param[in]  VariableName          Name of the variable to be found.
  @param[in]  VendorGuid            Variable vendor GUID to be found.
  @param[out] VariableInfo          Pointer to AUTH_VARIABLE_INFO structure for
                                    output of the next variable.

  @retval EFI_INVALID_PARAMETER     If VariableName is not an empty string,
                                    while VendorGuid is NULL.
  @retval EFI_SUCCESS               Variable successfully found.
  @retval EFI_NOT_FOUND             Variable not found

**/
EFI_STATUS
EFIAPI
VariableExLibFindNextVariable (
  IN  CHAR16                *VariableName,
  IN  EFI_GUID              *VendorGuid,
  OUT AUTH_VARIABLE_INFO    *VariableInfo
  )
{
  EFI_STATUS       Status;
  AUTH_VAR_HEADER *Header;

  Status = EmuGetNextVariableInternal (
             VariableName,
             VendorGuid,
             &Header,
             &mVariableModuleGlobal->VariableGlobal
             );
  if (EFI_ERROR (Status)) {
    VariableInfo->VariableName = NULL;
    VariableInfo->VendorGuid = NULL;
    VariableInfo->Data = NULL;
    VariableInfo->DataSize = 0;
    VariableInfo->Attributes = 0;
    VariableInfo->PubKeyIndex = 0;
    VariableInfo->MonotonicCount = 0;
    VariableInfo->TimeStamp = NULL;
    return Status;
  }

  VariableInfo->VariableName    = GetVariableNamePtr (Header);
  VariableInfo->VendorGuid      = &Header->VendorGuid;
  VariableInfo->DataSize        = Header->DataSize;
  VariableInfo->Data            = GetVariableDataPtr (Header);
  VariableInfo->Attributes      = Header->Attributes;
  VariableInfo->PubKeyIndex     = Header->PubKeyIndex;
  VariableInfo->MonotonicCount  = ReadUnaligned64 (&(Header->MonotonicCount));
  VariableInfo->TimeStamp       = &Header->TimeStamp;

  return EFI_SUCCESS;
}

/**
  Update the variable region with Variable information.

  @param[in] VariableInfo           Pointer AUTH_VARIABLE_INFO structure for
                                    input of the variable.

  @retval EFI_SUCCESS               The update operation is success.
  @retval EFI_INVALID_PARAMETER     Invalid parameter.
  @retval EFI_WRITE_PROTECTED       Variable is write-protected.
  @retval EFI_OUT_OF_RESOURCES      There is not enough resource.

**/
EFI_STATUS
EFIAPI
VariableExLibUpdateVariable (
  IN AUTH_VARIABLE_INFO     *VariableInfo
  )
{
  VARIABLE_POINTER_TRACK    Variable;

  FindVariable (
      VariableInfo->VariableName,
      VariableInfo->VendorGuid,
      &Variable,
      &mVariableModuleGlobal->VariableGlobal
      );
  return UpdateVariable (
           VariableInfo->VariableName,
           VariableInfo->VendorGuid,
           VariableInfo->Data,
           VariableInfo->DataSize,
           VariableInfo->Attributes,
           VariableInfo->PubKeyIndex,
           VariableInfo->MonotonicCount,
           &Variable,
           VariableInfo->TimeStamp,
           FALSE
           );
}

/**
  Get scratch buffer.

  @param[in, out] ScratchBufferSize Scratch buffer size. If input size is greater than
                                    the maximum supported buffer size, this value contains
                                    the maximum supported buffer size as output.
  @param[out]     ScratchBuffer     Pointer to scratch buffer address.

  @retval EFI_SUCCESS       Get scratch buffer successfully.
  @retval EFI_UNSUPPORTED   If input size is greater than the maximum supported buffer size.

**/
EFI_STATUS
EFIAPI
VariableExLibGetScratchBuffer (
  IN OUT UINTN      *ScratchBufferSize,
  OUT    VOID       **ScratchBuffer
  )
{
  UINTN MaxBufferSize;

  MaxBufferSize = mVariableModuleGlobal->ScratchBufferSize;
  if (*ScratchBufferSize > MaxBufferSize) {
    *ScratchBufferSize = MaxBufferSize;
    return EFI_UNSUPPORTED;
  }

  *ScratchBuffer =
    GetEndPointer ((VARIABLE_STORE_HEADER *)
                   mVariableModuleGlobal->VariableGlobal.VolatileVariableBase);
  return EFI_SUCCESS;
}

/**
  This function is to check if the remaining variable space is enough to set
  all Variables from argument list successfully. The purpose of the check
  is to keep the consistency of the Variables to be in variable storage.

  Note: Variables are assumed to be in same storage.
  The set sequence of Variables will be same with the sequence of VariableEntry from argument list,
  so follow the argument sequence to check the Variables.

  @param[in] Attributes         Variable attributes for Variable entries.
  @param ...                    The variable argument list with type VARIABLE_ENTRY_CONSISTENCY *.
                                A NULL terminates the list. The VariableSize of
                                VARIABLE_ENTRY_CONSISTENCY is the variable data size as input.
                                It will be changed to variable total size as output.

  @retval TRUE                  Have enough variable space to set the Variables successfully.
  @retval FALSE                 No enough variable space to set the Variables successfully.

**/
BOOLEAN
EFIAPI
VariableExLibCheckRemainingSpaceForConsistency (
  IN UINT32                     Attributes,
  ...
  )
{
  VA_LIST Marker;
  BOOLEAN Return;

  VA_START (Marker, Attributes);

  Return = CheckRemainingSpaceForConsistencyInternal (Attributes, Marker);

  VA_END (Marker);

  return Return;
}

/**
  Return TRUE if at OS runtime.

  @retval TRUE If at OS runtime.
  @retval FALSE If at boot time.

**/
BOOLEAN
EFIAPI
VariableExLibAtRuntime (
  VOID
  )
{
  return EfiAtRuntime ();
}

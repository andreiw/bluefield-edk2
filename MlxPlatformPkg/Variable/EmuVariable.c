/** @file

  Emulation Variable services operate on the runtime volatile memory.
  The nonvolatile variable space doesn't exist.

Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Variable.h"

///
/// Whether we've finished VariableCommonInitialize.
///
BOOLEAN mVariableCommonIsInitialized;

///
/// Whether we've initialized authenticated variables.
///
BOOLEAN mAuthenticatedVariablesAreInitialized;

///
/// Don't use module globals after the SetVirtualAddress map is signaled
///
VARIABLE_MODULE_GLOBAL *mVariableModuleGlobal;

VARIABLE_INFO_ENTRY *gVariableInfo = NULL;

VARIABLE_ENTRY_PROPERTY mVariableEntryProperty[] = {
  {
    &gEdkiiVarErrorFlagGuid,
    VAR_ERROR_FLAG_NAME,
    {
      VAR_CHECK_VARIABLE_PROPERTY_REVISION,
      VAR_CHECK_VARIABLE_PROPERTY_READ_ONLY,
      VARIABLE_ATTRIBUTE_NV_BS_RT,
      sizeof (VAR_ERROR_FLAG),
      sizeof (VAR_ERROR_FLAG)
    }
  },
};

AUTH_VAR_LIB_CONTEXT_IN mAuthContextIn = {
  AUTH_VAR_LIB_CONTEXT_IN_STRUCT_VERSION,
  sizeof (AUTH_VAR_LIB_CONTEXT_IN),
  0, // MaxAuthVariableSize; to be filled.
  VariableExLibFindVariable,
  VariableExLibFindNextVariable,
  VariableExLibUpdateVariable,
  VariableExLibGetScratchBuffer,
  VariableExLibCheckRemainingSpaceForConsistency,
  VariableExLibAtRuntime,
};

AUTH_VAR_LIB_CONTEXT_OUT mAuthContextOut;

/**

  This code gets the current status of Variable Store.

  @param VarStoreHeader  Pointer to the Variable Store Header.

  @retval EfiRaw         Variable store status is raw.
  @retval EfiValid       Variable store status is valid.
  @retval EfiInvalid     Variable store status is invalid.

**/
VARIABLE_STORE_STATUS
GetVariableStoreStatus (
  IN VARIABLE_STORE_HEADER *VarStoreHeader
  )
{
  if (CompareGuid (&VarStoreHeader->Signature, &gEfiAuthenticatedVariableGuid) &&
      VarStoreHeader->Format == VARIABLE_STORE_FORMATTED &&
      VarStoreHeader->State == VARIABLE_STORE_HEALTHY
      ) {

    return EfiValid;
  } else if (((UINT32 *)(&VarStoreHeader->Signature))[0] == 0xffffffff &&
             ((UINT32 *)(&VarStoreHeader->Signature))[1] == 0xffffffff &&
             ((UINT32 *)(&VarStoreHeader->Signature))[2] == 0xffffffff &&
             ((UINT32 *)(&VarStoreHeader->Signature))[3] == 0xffffffff &&
             VarStoreHeader->Size == 0xffffffff &&
             VarStoreHeader->Format == 0xff &&
             VarStoreHeader->State == 0xff
          ) {

    return EfiRaw;
  } else {
    return EfiInvalid;
  }
}

/**
  Acquires lock only at boot time. Simply returns at runtime.

  This is a temporary function which will be removed when
  EfiAcquireLock() in UefiLib can handle the call in UEFI
  Runtimer driver in RT phase.
  It calls EfiAcquireLock() at boot time, and simply returns
  at runtime

  @param  Lock         A pointer to the lock to acquire

**/
VOID
AcquireLockOnlyAtBootTime (
  IN EFI_LOCK  *Lock
  )
{
  if (!EfiAtRuntime ()) {
    EfiAcquireLock (Lock);
  }
}

/**
  Releases lock only at boot time. Simply returns at runtime.

  This is a temporary function which will be removed when
  EfiReleaseLock() in UefiLib can handle the call in UEFI
  Runtimer driver in RT phase.
  It calls EfiReleaseLock() at boot time, and simply returns
  at runtime

  @param  Lock         A pointer to the lock to release

**/
VOID
ReleaseLockOnlyAtBootTime (
  IN EFI_LOCK  *Lock
  )
{
  if (!EfiAtRuntime ()) {
    EfiReleaseLock (Lock);
  }
}

/**
  This code gets the pointer to the variable name.

  @param Variable        Pointer to the Variable Header.

  @return Pointer to Variable Name which is Unicode encoding.

**/
CHAR16 *
GetVariableNamePtr (
  IN  AUTH_VAR_HEADER   *Variable
  )
{
  return (CHAR16 *) ((UINTN) Variable + sizeof (AUTH_VAR_HEADER));
}

/**

  This code checks if variable header is valid or not.

  @param Variable           Pointer to the Variable Header.
  @param VariableStoreEnd   Pointer to the Variable Store End.

  @retval TRUE              Variable header is valid.
  @retval FALSE             Variable header is not valid.

**/
BOOLEAN
IsValidVariableHeader (
  IN  AUTH_VAR_HEADER       *Variable,
  IN  AUTH_VAR_HEADER       *VariableStoreEnd
  )
{
  return Variable != NULL && Variable < VariableStoreEnd && Variable->StartId == VARIABLE_DATA;
}

/**
  Gets pointer to the variable data.

  This function gets the pointer to the variable data according
  to the input pointer to the variable header.

  @param  Variable      Pointer to the variable header.

  @return Pointer to variable data

**/
UINT8 *
GetVariableDataPtr (
  IN  AUTH_VAR_HEADER   *Variable
  )
{
  //
  // Be careful about pad size for alignment
  //
  return (UINT8 *) ((UINTN) GetVariableNamePtr (Variable) +
                    Variable->NameSize +
                    GET_PAD_SIZE (Variable->NameSize));
}

/**
  This code gets the variable data offset related to variable header.

  @param Variable        Pointer to the Variable Header.

  @return Variable Data offset.

**/
UINTN
GetVariableDataOffset (
  IN  AUTH_VAR_HEADER   *Variable
  )
{
  //
  // Be careful about pad size for alignment
  //
  return (sizeof (AUTH_VAR_HEADER) +
          Variable->NameSize +
          GET_PAD_SIZE (Variable->NameSize));
}

/**
  Gets pointer to header of the next potential variable.

  This function gets the pointer to the next potential variable header
  according to the input point to the variable header.  The return value
  is not a valid variable if the input variable was the last variable
  in the variabl store.

  @param  Variable      Pointer to header of the next variable

  @return Pointer to next variable header.
  @retval NULL  Input was not a valid variable header.

**/
AUTH_VAR_HEADER *
GetNextPotentialVariablePtr (
  IN  AUTH_VAR_HEADER   *Variable
  )
{
  UINTN Value;

  if (Variable->StartId != VARIABLE_DATA) {
    return NULL;
  }

  Value =  (UINTN) GetVariableDataPtr (Variable);
  Value += Variable->DataSize;
  Value += GET_PAD_SIZE (Variable->DataSize);

  //
  // Be careful about pad size for alignment
  //
  return (AUTH_VAR_HEADER *) HEADER_ALIGN (Value);
}

/**
  Gets pointer to header of the next variable.

  This function gets the pointer to the next variable header according
  to the input point to the variable header.

  @param  Variable      Pointer to header of the next variable

  @return Pointer to next variable header.

**/
AUTH_VAR_HEADER *
GetNextVariablePtr (
  IN  AUTH_VAR_HEADER   *Variable
  )
{
  AUTH_VAR_HEADER *VarHeader;

  VarHeader = GetNextPotentialVariablePtr (Variable);

  if ((VarHeader == NULL) || (VarHeader->StartId != VARIABLE_DATA)) {
    return NULL;
  }

  return VarHeader;
}

/**
  Print the variable store header.

  @param Prefix    A prefix string to print before the header.
  @param Header    Pointer to the variable store header.

**/
VOID
PrintVariableStoreHeader (
  IN CHAR8                  *Prefix,
  IN VARIABLE_STORE_HEADER  *Header
  )
{
  DEBUG ((EFI_D_VARIABLE, "%a Guid %g Size 0x%x Format 0x%x State 0x%x\n",
          Prefix, &Header->Signature, Header->Size, Header->Format, Header->State));
}

/**
  Print the variable store header and the variables the store
  contains.

  @param Prefix    A prefix string to print before the variable store.
  @param Store     Pointer to the variable store.

**/
VOID
PrintVariableStore (
  IN CHAR8  *Prefix,
  IN VOID   *Store)
{
  VARIABLE_STORE_HEADER *Header;
  AUTH_VAR_HEADER       *Variable;

  Header = (VARIABLE_STORE_HEADER *)Store;

  if (GetVariableStoreStatus (Header) != EfiValid) {
    DEBUG ((EFI_D_VARIABLE, "%a: Variable Store not valid\n", Prefix));
    return;
  }

  PrintVariableStoreHeader (Prefix, Header);

  Variable = (AUTH_VAR_HEADER *)((UINT8 *)Store + sizeof (VARIABLE_STORE_HEADER));

  while (Variable != NULL && Variable->StartId == VARIABLE_DATA) {
    DEBUG ((EFI_D_VARIABLE, "Variable %s State 0x%x Attributes 0x%x\n",
            GetVariableNamePtr (Variable), Variable->State, Variable->Attributes));

    Variable = GetNextPotentialVariablePtr (Variable);
  }
}

/**
  Updates LastVariableOffset variable for the given variable store.

  LastVariableOffset points to the offset to use for the next variable
  when updating the variable store.

  @param[in]   VariableStore       Pointer to the start of the variable store
  @param[out]  LastVariableOffset  Offset to put the next new variable in

**/
VOID
InitializeLocationForLastVariableOffset (
  IN  VARIABLE_STORE_HEADER *VariableStore,
  OUT UINTN                 *LastVariableOffset
  )
{
  AUTH_VAR_HEADER *Variable;
  AUTH_VAR_HEADER *NextVariable;
  UINTN            VariableSize;

  //
  // Parse non-volatile variable data and get last variable offset.
  //
  Variable  = GetStartPointer (VariableStore);
  while (IsValidVariableHeader (Variable, GetEndPointer (VariableStore))) {
    NextVariable = GetNextPotentialVariablePtr (Variable);
    VariableSize = (UINTN) NextVariable - (UINTN) Variable;
    if ((Variable->Attributes &
         (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) ==
           (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) {
      mVariableModuleGlobal->HwErrVariableTotalSize  += VariableSize;
    } else {
      mVariableModuleGlobal->CommonVariableTotalSize += VariableSize;
    }

    Variable = NextVariable;
  }

  *LastVariableOffset = (UINTN) Variable - (UINTN) VariableStore;
}

/**
  Gets the pointer to the first variable header in given variable store area.

  @param VarStoreHeader  Pointer to the Variable Store Header.

  @return Pointer to the first variable header.

**/
AUTH_VAR_HEADER *
GetStartPointer (
  IN VARIABLE_STORE_HEADER *Header
  )
{
  return (AUTH_VAR_HEADER *) HEADER_ALIGN (Header + 1);
}

/**
  Gets pointer to the end of the variable storage area.

  This function gets pointer to the end of the variable storage
  area, according to the input variable store header.

  @param  VolHeader     Pointer to the variale store header

  @return Pointer to the end of the variable storage area.

**/
AUTH_VAR_HEADER *
GetEndPointer (
  IN VARIABLE_STORE_HEADER       *VolHeader
  )
{
  //
  // The end of variable store
  //
  return (AUTH_VAR_HEADER *) HEADER_ALIGN ((UINTN) VolHeader + VolHeader->Size);
}

/**
  Returns whether a variable is in VAR_ADDED or
  VAR_IN_DELETED_TRANSITION state.

  @param  Variable     The variable.

  @return Whether the variable is live.

**/
BOOLEAN
VariableIsLive (
  IN AUTH_VAR_HEADER  *Variable
  )
{
  return (Variable->State == VAR_ADDED ||
          Variable->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED));
}

/**
  Authenticated variable format check.

  @param[in] Variable           Pointer to the variable to check.

  @retval EFI_SUCCESS           The Variable check result was success.
  @retval EFI_INVALID_PARAMETER An invalid combination of attribute bits, name,
                                GUID, DataSize and Data value was supplied.

**/
EFI_STATUS
VariableStoreCheck (
  IN AUTH_VAR_HEADER    *Variable
  )
{
  CHAR16 *VariableName;
  UINT8  *VariableData;

  //
  // Only do check after InitNvStore call.
  //

  //
  // Check input parameters
  //
  if (Variable == NULL) {
    DEBUG ((EFI_D_VARIABLE, "[Variable] Invalid variable\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check the variable header
  //
  if (Variable->StartId != VARIABLE_DATA) {
    DEBUG ((EFI_D_VARIABLE, "[Variable] Invalid StartId 0x%x\n",
                Variable->StartId));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for reserved bits in variable attribute.
  //
  if ((Variable->Attributes & ~EFI_VARIABLE_ATTRIBUTES_MASK) != 0) {
    DEBUG ((EFI_D_VARIABLE, "[Variable] Invalid Attributes 0x%x\n",
                Variable->Attributes));
    return EFI_INVALID_PARAMETER;
  }

  //
  // *TBD* Check the vendor Guid.
  //

  //
  // Check the variable name.
  //
  VariableName = GetVariableNamePtr (Variable);
  if (VariableName == NULL || VariableName[0] == 0) {
    DEBUG ((EFI_D_VARIABLE, "[Variable] Invalid name\n",
                  Variable->Attributes));
    return EFI_INVALID_PARAMETER;
  }

  if (Variable->NameSize != StrSize (VariableName)) {
    DEBUG ((EFI_D_VARIABLE, "[Variable] Invalid name size %u != %u\n",
                Variable->NameSize, StrSize (VariableName)));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check the variable data.
  //
  VariableData = GetVariableDataPtr (Variable);
  if (VariableData == NULL) {
    DEBUG ((EFI_D_VARIABLE, "[Variable] Invalid variable data\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // One way to check the data size is to verify whether the next variable
  // is valid. If next variable is NULL then either the variable is the last
  // one in the store or the next potential variable is corrupted. Anyway,
  // this later would be ignored; so don't check for the NextVariable and
  // assume the check result is success at this step. Note that the variable
  // data could be corrupted, though.
  //
  return EFI_SUCCESS;
}

/**
  Gets pointer to header of the next potential valid variable in the store.

  This function gets the pointer to the next potential variable header
  according to the input point to the variable header.  The caller is
  responsible of checking whether the return value is valid.  If the
  input variable was the last variable in the variable store, then the
  return value is not valid.

  @param[in] Variable           Pointer to header of the next variable.
  @param[in] VariableStoreEnd   Pointer to the Variable Store End.

  @return Pointer to next variable header.
  @retval NULL  Input was not a valid variable header.

**/
AUTH_VAR_HEADER *
LookupNextPotentialVariable (
  IN  AUTH_VAR_HEADER   *Variable,
  IN  AUTH_VAR_HEADER   *VariableStoreEnd
  )
{
  AUTH_VAR_HEADER   *NextVariable;
  UINT16            *VariableDataPtr;

  if (Variable->StartId != VARIABLE_DATA) {
    return NULL;
  }

  NextVariable = GetNextPotentialVariablePtr (Variable);
  VariableDataPtr = (UINT16 *) NextVariable;

  while (NextVariable < VariableStoreEnd) {
      if (*VariableDataPtr++ == VARIABLE_DATA)
          break;
      NextVariable = (AUTH_VAR_HEADER *) VariableDataPtr;
  }

  return NextVariable;
}

/**
  Check whether the variables the store contains are valid. Optionally,
  enable garbage collection and recovery operation.

  @param[in]  Prefix              A prefix string to print before the variable
                                  store.
  @param[in]  Store               Pointer to the variable store.
  @param[in]  VariableRestore     Whether to restore the variable store
                                  content.


  @retval EFI_SUCCESS           The Variable check result was success.
  @retval EFI_INVALID_PARAMETER An invalid combination of store header and
                                store data was supplied.
  @retval EFI_NOT_FOUND         A default Variable was not found.
  @retval EFI_VOLUME_CORRUPTED  Duplicated valid entries of the given variable
                                exist in the store. Thus the store is marked
                                as corrupted.

**/
EFI_STATUS
VariableStoreCheckAndRestore (
  IN     CHAR8      *Prefix,
  IN     VOID       *Store,
  IN     BOOLEAN    VariableRestore
  )
{
  EFI_STATUS            Status = EFI_SUCCESS;
  VARIABLE_STORE_HEADER *Header;
  AUTH_VAR_HEADER       *Variable;
  AUTH_VAR_HEADER       *NextVariable;
  AUTH_VAR_HEADER       *CurrVariable;
  UINT8                 *VariableCache;
  UINT8                 *CurrPtr;
  UINTN                 Size, Index;
  BOOLEAN               NonVolatileVarExists;
  UINTN                 VariableCheckErr = 0;

  Header = (VARIABLE_STORE_HEADER *) Store;
  Size   = Header->Size;
  DEBUG ((EFI_D_VARIABLE, "%a: Checking Variable Store size 0x%x\n",
            Prefix, Size));

  //
  // This is unlikely to be true; The caller might check if the store
  // header is valid before calling this function.
  //
  if (GetVariableStoreStatus (Header) != EfiValid) {
    DEBUG ((EFI_D_ERROR, "%a: Variable Store header not valid\n",
                Prefix));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Allocate a temporary buffer large enough to hold all the
  // variables.  This buffer is used for recovery, if needed.
  //
  ASSERT (Size > sizeof (VARIABLE_STORE_HEADER));
  VariableCache = (UINT8 *) AllocatePool (Size);
  ASSERT (VariableCache != NULL);
  SetMem (VariableCache, Size, 0xff);

  //
  // Copy the store header.
  //
  CopyMem (VariableCache, Header, sizeof (VARIABLE_STORE_HEADER));
  CurrPtr = (UINT8 *) GetStartPointer ((VARIABLE_STORE_HEADER *)VariableCache);

  DEBUG ((EFI_D_ERROR, "%a: Checking variable format.\n", Prefix));

  //
  // Check the variable format.
  //
  Variable = GetStartPointer ((VARIABLE_STORE_HEADER *)Store);
  while (IsValidVariableHeader (Variable, GetEndPointer (Header))) {
    Status       = VariableStoreCheck (Variable);
    NextVariable = LookupNextPotentialVariable (Variable,
                                                GetEndPointer (Header));
    if (EFI_ERROR (Status)) {
      VariableCheckErr++;
    } else if (Variable->State == VAR_ADDED) {
      //
      // Keep all ADDED variables in the cache. If a recovery is needed
      // then swap the cache to the store.
      //
      Size = (UINTN) GetNextPotentialVariablePtr (Variable) - (UINTN) Variable;
      CopyMem (CurrPtr, Variable, Size);
      CurrPtr += Size;
    }
    Variable = NextVariable;
  }

  if (VariableCheckErr != 0) {
    DEBUG ((EFI_D_VARIABLE, "%a: Variable Store not valid: %u errors\n",
                Prefix, VariableCheckErr));
  }

  DEBUG ((EFI_D_ERROR, "%a: Checking default variables.\n", Prefix));

  //
  // Check for default variables. After the first boot, the system
  // requires at least these variables in order to run properly.
  // Careful, this check has been added to prevent potential issues
  // later in the boot process. Plus some configuration might not
  // require all of these variables. This might slow down the boot
  // process if several variables are saved to the store.
  //
  CHAR16 *NonVolatileVarNames[] = {
    L"AuthVarKeyDatabase",
    L"CustomMode",
    L"certdb",
    L"VendorKeysNv",
    L"RshimMacAddr",
    L"Timeout",
    L"PlatformLang",
    L"Lang",
    L"ConOut",
    L"ConIn",
    L"ErrOut",
    L"MemoryTypeInformation"
  };
  for (Index = 0;
       Index < sizeof (NonVolatileVarNames)/sizeof (NonVolatileVarNames[0]);
       Index++) {
    Variable = GetStartPointer ((VARIABLE_STORE_HEADER *)VariableCache);
    NonVolatileVarExists = FALSE;
    while ((NextVariable = GetNextPotentialVariablePtr (Variable)) != NULL &&
              Variable->StartId == VARIABLE_DATA) {
      if (Variable->State == VAR_ADDED &&
          Variable->NameSize == StrSize(NonVolatileVarNames[Index]) &&
          CompareMem (GetVariableNamePtr (Variable),
                      NonVolatileVarNames[Index],
                      Variable->NameSize) == 0) {
         NonVolatileVarExists = TRUE;
         Variable             = NextVariable;
         break;
       }
      Variable = NextVariable;
    }
    if (!NonVolatileVarExists) {
      DEBUG ((EFI_D_VARIABLE, "%a: Failed to find variable %s\n",
                  Prefix, NonVolatileVarNames[Index]));
      FreePool (VariableCache);
      return EFI_NOT_FOUND;
    }
  }

  DEBUG ((EFI_D_ERROR, "%a: Checking duplicated variables.\n", Prefix));

  //
  // Check for duplicated variables and delete them before re-installing
  // all valid variables to the store. This might slow down the boot process
  // if several variables are saved to the store.
  //
  Variable = GetStartPointer ((VARIABLE_STORE_HEADER *)VariableCache);
  while (Variable != NULL && Variable->StartId == VARIABLE_DATA) {
    if (Variable->State == VAR_ADDED) {
      CurrVariable = Variable;
      while ((CurrVariable =
                GetNextPotentialVariablePtr (CurrVariable)) != NULL &&
                    ((UINTN) CurrPtr - (UINTN) CurrVariable > 0)) {
        if (CurrVariable->State == VAR_ADDED &&
            CurrVariable->NameSize == Variable->NameSize &&
            CompareMem (GetVariableNamePtr (Variable),
                        GetVariableNamePtr (CurrVariable),
                        Variable->NameSize) == 0) {
            DEBUG ((EFI_D_VARIABLE,
                      "%a: Variable %s has duplicate: deleting\n",
                      Prefix, GetVariableNamePtr (Variable)));
            //
            // Mark the variable as deleted.
            //
            Variable->State &= VAR_IN_DELETED_TRANSITION;
            Variable->State &= VAR_DELETED;
            Status           = EFI_VOLUME_CORRUPTED;
            break;
        }
      }
    }
    Variable = GetNextPotentialVariablePtr (Variable);
  }

  //
  // Return if either the variable store is valid or the store recovery
  // isn't needed.
  //
  if (!VariableRestore ||
        (VariableCheckErr == 0 && Status != EFI_VOLUME_CORRUPTED)) {
      DEBUG ((EFI_D_VARIABLE, "%a: Variable Store is valid\n", Prefix));
      FreePool (VariableCache);
      return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_VARIABLE, "%a: Variable store recovery.\n", Prefix));

  //
  // Copy buffer to variable store.
  //
  SetMem (Store, Header->Size, 0xff);
  CopyMem (Store, VariableCache, CurrPtr - VariableCache);

  FreePool (VariableCache);

  return Status;
}

/**

  Variable store garbage collection and reclaim operation.

  @param[in]      VariableBase            Base address of variable store.
  @param[out]     LastVariableOffset      Offset of last variable.
  @param[in]      IsVolatile              Whether the variable store is volatile.
  @param[in, out] UpdatingPtrTrack        Pointer to updating variable pointer track structure.
  @param[in]      DeleteUpdating          Whether to delete the variable in UpdatingPtrTrack.

  @retval EFI_SUCCESS                  Reclaim operation has finished successfully.
  @retval EFI_OUT_OF_RESOURCES         No enough memory resources or variable space.
  @retval Others                       Unexpected error happened during reclaim operation.

**/
EFI_STATUS
Reclaim (
  IN     EFI_PHYSICAL_ADDRESS         VariableBase,
  OUT    UINTN                       *LastVariableOffset,
  IN     BOOLEAN                      IsVolatile,
  IN OUT VARIABLE_POINTER_TRACK      *UpdatingPtrTrack,
  IN     BOOLEAN                      DeleteUpdating
  )
{
  AUTH_VAR_HEADER       *Variable;
  AUTH_VAR_HEADER       *NextVariable;
  AUTH_VAR_HEADER       *AddedVariable;
  AUTH_VAR_HEADER       *NextAddedVariable;

  VARIABLE_STORE_HEADER *VariableStoreHeader;
  UINT8                 *Store;
  VARIABLE_STORE_HEADER *StoreHeader;
  UINTN                  Size;
  UINTN                  NameSize;
  UINT8                 *CurrPtr;
  BOOLEAN                FoundAdded;
  UINT8                 *Name0;
  UINT8                 *Name1;
  EFI_STATUS             Status;
  UINTN                  CommonVariableTotalSize;
  UINTN                  HwErrVariableTotalSize;

  VariableStoreHeader = (VARIABLE_STORE_HEADER *) VariableBase;

  CommonVariableTotalSize = 0;
  HwErrVariableTotalSize  = 0;

  //
  // Allocate a temporary buffer large enough to hold all the
  // variables.  Add 1 byte for the terminating 0xff to identify the
  // end of buffer.  Note this bound is conservative if DeleteUpdating
  // is true, but it's not worth accounting for.
  //
  Size = *LastVariableOffset + 1;
  DEBUG ((EFI_D_VARIABLE, "Variable Reclaim(): %a initial size %d\n",
          IsVolatile ? "Vol" : "NV", Size));
  Store = AllocatePool (Size);
  if (Store == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  SetMem (Store, Size, 0xff);
  StoreHeader = (VARIABLE_STORE_HEADER *) Store;

  //
  // Copy variable store header.
  //
  CopyMem (Store, VariableStoreHeader, sizeof (VARIABLE_STORE_HEADER));
  CurrPtr = (UINT8 *) GetStartPointer (StoreHeader);

  //
  // Reinstall all ADDED variables, except if DeleteUpdating is TRUE
  // then don't reinstall UpdatingPtrTrack->CurrPtr.
  //
  Variable = GetStartPointer (VariableStoreHeader);
  while (IsValidVariableHeader (Variable, GetEndPointer (VariableStoreHeader))) {
    NextVariable = GetNextPotentialVariablePtr (Variable);
    if (Variable->State == VAR_ADDED &&
        !(DeleteUpdating && (Variable == UpdatingPtrTrack->CurrPtr))) {
      Size = (UINTN) NextVariable - (UINTN) Variable;
      CopyMem (CurrPtr, Variable, Size);
      if (Variable == UpdatingPtrTrack->CurrPtr) {
        UpdatingPtrTrack->CurrPtr =
          (AUTH_VAR_HEADER *)((UINTN)UpdatingPtrTrack->StartPtr +
                              (CurrPtr - (UINT8 *)GetStartPointer (StoreHeader)));
      }
      CurrPtr += Size;
      if (!IsVolatile) {
        if ((Variable->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
          HwErrVariableTotalSize += Size;
        } else {
          CommonVariableTotalSize += Size;
        }
      }
    }
    Variable = NextVariable;
  }

  //
  // Reinstall all in delete transition variables, except if
  // DeleteUpdating is TRUE then don't reinstall UpdatingPtrTrack->CurrPtr.
  //
  Variable = GetStartPointer (VariableStoreHeader);
  while (IsValidVariableHeader (Variable, GetEndPointer (VariableStoreHeader))) {
    NextVariable = GetNextVariablePtr (Variable);
    if (Variable->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED) &&
        !(DeleteUpdating && (Variable == UpdatingPtrTrack->CurrPtr))) {

      //
      // We have copied all ADDED variables, so we only keep IN_DELETED
      // variables that we can't find there.
      //

      FoundAdded = FALSE;
      AddedVariable = GetStartPointer (StoreHeader);
      while (IsValidVariableHeader (AddedVariable, GetEndPointer (StoreHeader))) {
        NextAddedVariable = GetNextVariablePtr (AddedVariable);
        NameSize = AddedVariable->NameSize;
        if (CompareGuid (&AddedVariable->VendorGuid, &Variable->VendorGuid) &&
            NameSize == Variable->NameSize) {
          Name0 = (VOID *) GetVariableNamePtr (AddedVariable);
          Name1 = (VOID *) GetVariableNamePtr (Variable);
          if (CompareMem (Name0, Name1, NameSize) == 0) {
            FoundAdded = TRUE;
            break;
          }
        }
        AddedVariable = NextAddedVariable;
      }
      if (!FoundAdded) {
        //
        // Promote VAR_IN_DELETED_TRANSITION to VAR_ADDED.
        //
        Size = (UINTN) NextVariable - (UINTN) Variable;
        CopyMem (CurrPtr, Variable, Size);
        ((AUTH_VAR_HEADER *) CurrPtr)->State = VAR_ADDED;
        if (Variable == UpdatingPtrTrack->CurrPtr) {
          UpdatingPtrTrack->CurrPtr =
            (AUTH_VAR_HEADER *)((UINTN)UpdatingPtrTrack->StartPtr +
                                (CurrPtr - (UINT8 *)GetStartPointer (StoreHeader)));
        }
        CurrPtr += Size;
        if (!IsVolatile) {
          if ((Variable->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
            HwErrVariableTotalSize += Size;
          } else {
            CommonVariableTotalSize += Size;
          }
        }
      }
    }

    Variable = NextVariable;
  }

  if (UpdatingPtrTrack != NULL) {
    if (DeleteUpdating) {
      UpdatingPtrTrack->CurrPtr = NULL;
    }
    UpdatingPtrTrack->InDeletedTransitionPtr = NULL;
  }

  //
  // Copy buffer to variable base.
  //
  Status = EFI_SUCCESS;
  SetMem (VariableStoreHeader, VariableStoreHeader->Size, 0xff);
  CopyMem (VariableStoreHeader, Store, CurrPtr - Store);
  *LastVariableOffset = CurrPtr - Store;
  if (!IsVolatile) {
    mVariableModuleGlobal->HwErrVariableTotalSize = HwErrVariableTotalSize;
    mVariableModuleGlobal->CommonVariableTotalSize = CommonVariableTotalSize;
  }

  //
  // Copy result to NV store.
  //
  if (IsActiveNvStore () && !IsVolatile) {
    ReclaimNvStore (Store, CurrPtr - Store);
  }

  DEBUG ((EFI_D_VARIABLE, "Variable Reclaim(): %a final size %d\n",
          IsVolatile ? "Vol" : "NV", *LastVariableOffset));

  FreePool (Store);

  return Status;
}

/**
  Routine used to track statistical information about variable usage. 
  The data is stored in the EFI system table so it can be accessed later.
  VariableInfo.efi can dump out the table. Only Boot Services variable 
  accesses are tracked by this code. The PcdVariableCollectStatistics
  build flag controls if this feature is enabled. 

  A read that hits in the cache will have Read and Cache true for 
  the transaction. Data is allocated by this routine, but never
  freed.

  @param[in] VariableName   Name of the Variable to track
  @param[in] VendorGuid     Guid of the Variable to track
  @param[in] Volatile       TRUE if volatile FALSE if non-volatile
  @param[in] Read           TRUE if GetVariable() was called
  @param[in] Write          TRUE if SetVariable() was called
  @param[in] Delete         TRUE if deleted via SetVariable()
  @param[in] Cache          TRUE for a cache hit.

**/
VOID
UpdateVariableInfo (
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  IN  BOOLEAN                 Volatile,
  IN  BOOLEAN                 Read,
  IN  BOOLEAN                 Write,
  IN  BOOLEAN                 Delete,
  IN  BOOLEAN                 Cache
  )
{
  VARIABLE_INFO_ENTRY   *Entry;

  if (FeaturePcdGet (PcdVariableCollectStatistics)) {

    if (EfiAtRuntime ()) {
      // Don't collect statistics at runtime
      return;
    }

    if (gVariableInfo == NULL) {
      //
      // on the first call allocate a entry and place a pointer to it in
      // the EFI System Table
      //
      gVariableInfo = AllocateZeroPool (sizeof (VARIABLE_INFO_ENTRY));
      ASSERT (gVariableInfo != NULL);

      CopyGuid (&gVariableInfo->VendorGuid, VendorGuid);
      gVariableInfo->Name = AllocateZeroPool (StrSize (VariableName));
      ASSERT (gVariableInfo->Name != NULL);
      StrCpyS (gVariableInfo->Name, StrSize(VariableName)/sizeof(CHAR16), VariableName);
      gVariableInfo->Volatile = Volatile;

      gBS->InstallConfigurationTable (&gEfiAuthenticatedVariableGuid, gVariableInfo);
    }

    
    for (Entry = gVariableInfo; Entry != NULL; Entry = Entry->Next) {
      if (CompareGuid (VendorGuid, &Entry->VendorGuid)) {
        if (StrCmp (VariableName, Entry->Name) == 0) {
          if (Read) {
            Entry->ReadCount++;
          }
          if (Write) {
            Entry->WriteCount++;
          }
          if (Delete) {
            Entry->DeleteCount++;
          }
          if (Cache) {
            Entry->CacheCount++;
          }

          return;
        }
      }

      if (Entry->Next == NULL) {
        //
        // If the entry is not in the table add it.
        // Next iteration of the loop will fill in the data
        //
        Entry->Next = AllocateZeroPool (sizeof (VARIABLE_INFO_ENTRY));
        ASSERT (Entry->Next != NULL);

        CopyGuid (&Entry->Next->VendorGuid, VendorGuid);
        Entry->Next->Name = AllocateZeroPool (StrSize (VariableName));
        ASSERT (Entry->Next->Name != NULL);
        StrCpyS (Entry->Next->Name, StrSize(VariableName)/sizeof(CHAR16), VariableName);
        Entry->Next->Volatile = Volatile;
      }

    }
  }
}

/**
  Get index from supported language codes according to language string.

  This code is used to get corresponding index in supported language codes. It can handle
  RFC4646 and ISO639 language tags.
  In ISO639 language tags, take 3-characters as a delimitation to find matched string and calculate the index.
  In RFC4646 language tags, take semicolon as a delimitation to find matched string and calculate the index.

  For example:
    SupportedLang  = "engfraengfra"
    Lang           = "eng"
    Iso639Language = TRUE
  The return value is "0".
  Another example:
    SupportedLang  = "en;fr;en-US;fr-FR"
    Lang           = "fr-FR"
    Iso639Language = FALSE
  The return value is "3".

  @param  SupportedLang               Platform supported language codes.
  @param  Lang                        Configured language.
  @param  Iso639Language              A bool value to signify if the handler is operated on ISO639 or RFC4646.

  @retval the index of language in the language codes.

**/
UINTN
GetIndexFromSupportedLangCodes(
  IN  CHAR8            *SupportedLang,
  IN  CHAR8            *Lang,
  IN  BOOLEAN          Iso639Language
  ) 
{
  UINTN    Index;
  UINTN    CompareLength;
  UINTN    LanguageLength;

  if (Iso639Language) {
    CompareLength = ISO_639_2_ENTRY_SIZE;
    for (Index = 0; Index < AsciiStrLen (SupportedLang); Index += CompareLength) {
      if (AsciiStrnCmp (Lang, SupportedLang + Index, CompareLength) == 0) {
        //
        // Successfully find the index of Lang string in SupportedLang string.
        //
        Index = Index / CompareLength;
        return Index;
      }
    }
    ASSERT (FALSE);
    return 0;
  } else {
    //
    // Compare RFC4646 language code
    //
    Index = 0;
    for (LanguageLength = 0; Lang[LanguageLength] != '\0'; LanguageLength++);

    for (Index = 0; *SupportedLang != '\0'; Index++, SupportedLang += CompareLength) {
      //
      // Skip ';' characters in SupportedLang
      //
      for (; *SupportedLang != '\0' && *SupportedLang == ';'; SupportedLang++);
      //
      // Determine the length of the next language code in SupportedLang
      //
      for (CompareLength = 0; SupportedLang[CompareLength] != '\0' && SupportedLang[CompareLength] != ';'; CompareLength++);
      
      if ((CompareLength == LanguageLength) && 
          (AsciiStrnCmp (Lang, SupportedLang, CompareLength) == 0)) {
        //
        // Successfully find the index of Lang string in SupportedLang string.
        //
        return Index;
      }
    }
    ASSERT (FALSE);
    return 0;
  }
}

/**
  Get language string from supported language codes according to index.

  This code is used to get corresponding language string in supported language codes. It can handle
  RFC4646 and ISO639 language tags.
  In ISO639 language tags, take 3-characters as a delimitation. Find language string according to the index.
  In RFC4646 language tags, take semicolon as a delimitation. Find language string according to the index.

  For example:
    SupportedLang  = "engfraengfra"
    Index          = "1"
    Iso639Language = TRUE
  The return value is "fra".
  Another example:
    SupportedLang  = "en;fr;en-US;fr-FR"
    Index          = "1"
    Iso639Language = FALSE
  The return value is "fr".

  @param  SupportedLang               Platform supported language codes.
  @param  Index                       the index in supported language codes.
  @param  Iso639Language              A bool value to signify if the handler is operated on ISO639 or RFC4646.

  @retval the language string in the language codes.

**/
CHAR8 *
GetLangFromSupportedLangCodes (
  IN  CHAR8            *SupportedLang,
  IN  UINTN            Index,
  IN  BOOLEAN          Iso639Language
)
{
  UINTN    SubIndex;
  UINTN    CompareLength;
  CHAR8    *Supported;

  SubIndex  = 0;
  Supported = SupportedLang;
  if (Iso639Language) {
    //
    // according to the index of Lang string in SupportedLang string to get the language.
    // As this code will be invoked in RUNTIME, therefore there is not memory allocate/free operation.
    // In driver entry, it pre-allocates a runtime attribute memory to accommodate this string.
    //
    CompareLength = ISO_639_2_ENTRY_SIZE;
    mVariableModuleGlobal->Lang[CompareLength] = '\0';
    return CopyMem (mVariableModuleGlobal->Lang, SupportedLang + Index * CompareLength, CompareLength);

  } else {
    while (TRUE) {
      //
      // take semicolon as delimitation, sequentially traverse supported language codes.
      //
      for (CompareLength = 0; *Supported != ';' && *Supported != '\0'; CompareLength++) {
        Supported++;
      }
      if ((*Supported == '\0') && (SubIndex != Index)) {
        //
        // Have completed the traverse, but not find corrsponding string.
        // This case is not allowed to happen.
        //
        ASSERT(FALSE);
        return NULL;
      }
      if (SubIndex == Index) {
        //
        // according to the index of Lang string in SupportedLang string to get the language.
        // As this code will be invoked in RUNTIME, therefore there is not memory allocate/free operation.
        // In driver entry, it pre-allocates a runtime attribute memory to accommodate this string.
        //
        mVariableModuleGlobal->PlatformLang[CompareLength] = '\0';
        return CopyMem (mVariableModuleGlobal->PlatformLang, Supported - CompareLength, CompareLength);
      }
      SubIndex++;
      
      //
      // Skip ';' characters in Supported
      //
      for (; *Supported != '\0' && *Supported == ';'; Supported++);
    }
  }
}

/**
  Returns a pointer to an allocated buffer that contains the best matching language 
  from a set of supported languages.  
  
  This function supports both ISO 639-2 and RFC 4646 language codes, but language 
  code types may not be mixed in a single call to this function. This function
  supports a variable argument list that allows the caller to pass in a prioritized
  list of language codes to test against all the language codes in SupportedLanguages.

  If SupportedLanguages is NULL, then ASSERT().

  @param[in]  SupportedLanguages  A pointer to a Null-terminated ASCII string that
                                  contains a set of language codes in the format 
                                  specified by Iso639Language.
  @param[in]  Iso639Language      If TRUE, then all language codes are assumed to be
                                  in ISO 639-2 format.  If FALSE, then all language
                                  codes are assumed to be in RFC 4646 language format
  @param[in]  ...                 A variable argument list that contains pointers to 
                                  Null-terminated ASCII strings that contain one or more
                                  language codes in the format specified by Iso639Language.
                                  The first language code from each of these language
                                  code lists is used to determine if it is an exact or
                                  close match to any of the language codes in 
                                  SupportedLanguages.  Close matches only apply to RFC 4646
                                  language codes, and the matching algorithm from RFC 4647
                                  is used to determine if a close match is present.  If 
                                  an exact or close match is found, then the matching
                                  language code from SupportedLanguages is returned.  If
                                  no matches are found, then the next variable argument
                                  parameter is evaluated.  The variable argument list 
                                  is terminated by a NULL.

  @retval NULL   The best matching language could not be found in SupportedLanguages.
  @retval NULL   There are not enough resources available to return the best matching 
                 language.
  @retval Other  A pointer to a Null-terminated ASCII string that is the best matching 
                 language in SupportedLanguages.

**/
CHAR8 *
EFIAPI
VariableGetBestLanguage (
  IN CONST CHAR8  *SupportedLanguages, 
  IN BOOLEAN      Iso639Language,
  ...
  )
{
  VA_LIST      Args;
  CHAR8        *Language;
  UINTN        CompareLength;
  UINTN        LanguageLength;
  CONST CHAR8  *Supported;
  CHAR8        *Buffer;

  ASSERT (SupportedLanguages != NULL);

  VA_START (Args, Iso639Language);
  while ((Language = VA_ARG (Args, CHAR8 *)) != NULL) {
    //
    // Default to ISO 639-2 mode
    //
    CompareLength  = 3;
    LanguageLength = MIN (3, AsciiStrLen (Language));

    //
    // If in RFC 4646 mode, then determine the length of the first RFC 4646 language code in Language
    //
    if (!Iso639Language) {
      for (LanguageLength = 0; Language[LanguageLength] != 0 && Language[LanguageLength] != ';'; LanguageLength++);
    }

    //
    // Trim back the length of Language used until it is empty
    //
    while (LanguageLength > 0) {
      //
      // Loop through all language codes in SupportedLanguages
      //
      for (Supported = SupportedLanguages; *Supported != '\0'; Supported += CompareLength) {
        //
        // In RFC 4646 mode, then Loop through all language codes in SupportedLanguages
        //
        if (!Iso639Language) {
          //
          // Skip ';' characters in Supported
          //
          for (; *Supported != '\0' && *Supported == ';'; Supported++);
          //
          // Determine the length of the next language code in Supported
          //
          for (CompareLength = 0; Supported[CompareLength] != 0 && Supported[CompareLength] != ';'; CompareLength++);
          //
          // If Language is longer than the Supported, then skip to the next language
          //
          if (LanguageLength > CompareLength) {
            continue;
          }
        }
        //
        // See if the first LanguageLength characters in Supported match Language
        //
        if (AsciiStrnCmp (Supported, Language, LanguageLength) == 0) {
          VA_END (Args);

          Buffer = Iso639Language ? mVariableModuleGlobal->Lang : mVariableModuleGlobal->PlatformLang;
          Buffer[CompareLength] = '\0';
          return CopyMem (Buffer, Supported, CompareLength);
        }
      }

      if (Iso639Language) {
        //
        // If ISO 639 mode, then each language can only be tested once
        //
        LanguageLength = 0;
      } else {
        //
        // If RFC 4646 mode, then trim Language from the right to the next '-' character 
        //
        for (LanguageLength--; LanguageLength > 0 && Language[LanguageLength] != '-'; LanguageLength--);
      }
    }
  }
  VA_END (Args);

  //
  // No matches were found 
  //
  return NULL;
}

/**
  Hook the operations in PlatformLangCodes, LangCodes, PlatformLang and Lang.

  When setting Lang/LangCodes, simultaneously update PlatformLang/PlatformLangCodes.

  According to UEFI spec, PlatformLangCodes/LangCodes are only set once in firmware initialization,
  and are read-only. Therefore, in variable driver, only store the original value for other use.

  @param[in] VariableName       Name of variable

  @param[in] Data               Variable data

  @param[in] DataSize           Size of data. 0 means delete

**/
VOID
AutoUpdateLangVariable(
  IN  CHAR16             *VariableName,
  IN  VOID               *Data,
  IN  UINTN              DataSize
  )
{
  VARIABLE_GLOBAL        *Global;
  EFI_STATUS             Status;
  CHAR8                  *BestPlatformLang;
  CHAR8                  *BestLang;
  UINTN                  Index;
  UINT32                 Attributes;
  VARIABLE_POINTER_TRACK Variable;
  BOOLEAN                SetLanguageCodes;

  //
  // Don't do updates for delete operation
  //
  if (DataSize == 0) {
    return;
  }

  SetLanguageCodes = FALSE;

  Global = &mVariableModuleGlobal->VariableGlobal;

  if (StrCmp (VariableName, L"PlatformLangCodes") == 0) {
    //
    // PlatformLangCodes is a volatile variable, so it can not be updated at runtime.
    //
    if (EfiAtRuntime ()) {
      return;
    }

    SetLanguageCodes = TRUE;

    //
    // According to UEFI spec, PlatformLangCodes is only set once in firmware initialization, and is read-only
    // Therefore, in variable driver, only store the original value for other use.
    //
    if (mVariableModuleGlobal->PlatformLangCodes != NULL) {
      FreePool (mVariableModuleGlobal->PlatformLangCodes);
    }
    mVariableModuleGlobal->PlatformLangCodes = AllocateRuntimeCopyPool (DataSize, Data);
    ASSERT (mVariableModuleGlobal->PlatformLangCodes != NULL);

    //
    // PlatformLang holds a single language from PlatformLangCodes, 
    // so the size of PlatformLangCodes is enough for the PlatformLang.
    //
    if (mVariableModuleGlobal->PlatformLang != NULL) {
      FreePool (mVariableModuleGlobal->PlatformLang);
    }
    mVariableModuleGlobal->PlatformLang = AllocateRuntimePool (DataSize);
    ASSERT (mVariableModuleGlobal->PlatformLang != NULL);

  } else if (StrCmp (VariableName, L"LangCodes") == 0) {
    //
    // LangCodes is a volatile variable, so it can not be updated at runtime.
    //
    if (EfiAtRuntime ()) {
      return;
    }

    SetLanguageCodes = TRUE;

    //
    // According to UEFI spec, LangCodes is only set once in firmware initialization, and is read-only
    // Therefore, in variable driver, only store the original value for other use.
    //
    if (mVariableModuleGlobal->LangCodes != NULL) {
      FreePool (mVariableModuleGlobal->LangCodes);
    }
    mVariableModuleGlobal->LangCodes = AllocateRuntimeCopyPool (DataSize, Data);
    ASSERT (mVariableModuleGlobal->LangCodes != NULL);
  }

  if (SetLanguageCodes 
      && (mVariableModuleGlobal->PlatformLangCodes != NULL)
      && (mVariableModuleGlobal->LangCodes != NULL)) {
    //
    // Update Lang if PlatformLang is already set
    // Update PlatformLang if Lang is already set
    //
    Status = FindVariable (L"PlatformLang", &gEfiGlobalVariableGuid, &Variable, Global);
    if (!EFI_ERROR (Status)) {
      //
      // Update Lang
      //
      VariableName = L"PlatformLang";
      Data         = GetVariableDataPtr (Variable.CurrPtr);
      DataSize     = Variable.CurrPtr->DataSize;
    } else {
      Status = FindVariable (L"Lang", &gEfiGlobalVariableGuid, &Variable, Global);
      if (!EFI_ERROR (Status)) {
        //
        // Update PlatformLang
        //
        VariableName = L"Lang";
        Data         = GetVariableDataPtr (Variable.CurrPtr);
        DataSize     = Variable.CurrPtr->DataSize;
      } else {
        //
        // Neither PlatformLang nor Lang is set, directly return
        //
        return;
      }
    }
  }
  
  //
  // According to UEFI spec, "Lang" and "PlatformLang" is NV|BS|RT attributions.
  //
  Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

  if (StrCmp (VariableName, L"PlatformLang") == 0) {
    //
    // Update Lang when PlatformLangCodes/LangCodes were set.
    //
    if ((mVariableModuleGlobal->PlatformLangCodes != NULL) && (mVariableModuleGlobal->LangCodes != NULL)) {
      //
      // When setting PlatformLang, firstly get most matched language string from supported language codes.
      //
      BestPlatformLang = VariableGetBestLanguage (mVariableModuleGlobal->PlatformLangCodes, FALSE, Data, NULL);
      if (BestPlatformLang != NULL) {
        //
        // Get the corresponding index in language codes.
        //
        Index = GetIndexFromSupportedLangCodes (mVariableModuleGlobal->PlatformLangCodes, BestPlatformLang, FALSE);

        //
        // Get the corresponding ISO639 language tag according to RFC4646 language tag.
        //
        BestLang = GetLangFromSupportedLangCodes (mVariableModuleGlobal->LangCodes, Index, TRUE);

        //
        // Successfully convert PlatformLang to Lang, and set the BestLang value into Lang variable simultaneously.
        //
        FindVariable (L"Lang", &gEfiGlobalVariableGuid, &Variable, Global);

        Status = UpdateVariable (L"Lang", &gEfiGlobalVariableGuid, BestLang,
                                 ISO_639_2_ENTRY_SIZE + 1, Attributes, 0, 0, &Variable, NULL, FALSE);

        DEBUG ((EFI_D_INFO, "Variable Driver Auto Update PlatformLang, PlatformLang:%a, Lang:%a\n", BestPlatformLang, BestLang));

        ASSERT_EFI_ERROR(Status);
      }
    }

  } else if (StrCmp (VariableName, L"Lang") == 0) {
    //
    // Update PlatformLang when PlatformLangCodes/LangCodes were set.
    //
    if ((mVariableModuleGlobal->PlatformLangCodes != NULL) && (mVariableModuleGlobal->LangCodes != NULL)) {
      //
      // When setting Lang, firstly get most matched language string from supported language codes.
      //
      BestLang = VariableGetBestLanguage (mVariableModuleGlobal->LangCodes, TRUE, Data, NULL);
      if (BestLang != NULL) {
        //
        // Get the corresponding index in language codes.
        //
        Index = GetIndexFromSupportedLangCodes (mVariableModuleGlobal->LangCodes, BestLang, TRUE);

        //
        // Get the corresponding RFC4646 language tag according to ISO639 language tag.
        //
        BestPlatformLang = GetLangFromSupportedLangCodes (mVariableModuleGlobal->PlatformLangCodes, Index, FALSE);

        //
        // Successfully convert Lang to PlatformLang, and set the BestPlatformLang value into PlatformLang variable simultaneously.
        //
        FindVariable (L"PlatformLang", &gEfiGlobalVariableGuid, &Variable, Global);

        Status = UpdateVariable (L"PlatformLang", &gEfiGlobalVariableGuid, BestPlatformLang,
                                 AsciiStrSize (BestPlatformLang), Attributes, 0, 0, &Variable, NULL, FALSE);

        DEBUG ((EFI_D_INFO, "Variable Driver Auto Update Lang, Lang:%a, PlatformLang:%a\n", BestLang, BestPlatformLang));
        ASSERT_EFI_ERROR (Status);
      }
    }
  }
}

/**
  Compare two EFI_TIME data.


  @param FirstTime           A pointer to the first EFI_TIME data.
  @param SecondTime          A pointer to the second EFI_TIME data.

  @retval  TRUE              The FirstTime is not later than the SecondTime.
  @retval  FALSE             The FirstTime is later than the SecondTime.

**/
BOOLEAN
VariableCompareTimeStampInternal (
  IN EFI_TIME               *FirstTime,
  IN EFI_TIME               *SecondTime
  )
{
  if (FirstTime->Year != SecondTime->Year) {
    return (BOOLEAN) (FirstTime->Year < SecondTime->Year);
  } else if (FirstTime->Month != SecondTime->Month) {
    return (BOOLEAN) (FirstTime->Month < SecondTime->Month);
  } else if (FirstTime->Day != SecondTime->Day) {
    return (BOOLEAN) (FirstTime->Day < SecondTime->Day);
  } else if (FirstTime->Hour != SecondTime->Hour) {
    return (BOOLEAN) (FirstTime->Hour < SecondTime->Hour);
  } else if (FirstTime->Minute != SecondTime->Minute) {
    return (BOOLEAN) (FirstTime->Minute < SecondTime->Minute);
  }

  return (BOOLEAN) (FirstTime->Second <= SecondTime->Second);
}

/**
  Initialize authenticated variables.

**/
EFI_STATUS
InitAuthenticatedVariables (
  )
{
  EFI_STATUS              Status;
  UINTN                   Index;
  VARIABLE_ENTRY_PROPERTY *VariableEntry;

  if (mAuthenticatedVariablesAreInitialized) {
    return EFI_SUCCESS;
  }

  mAuthenticatedVariablesAreInitialized = TRUE;

  mAuthContextIn.MaxAuthVariableSize =
    mVariableModuleGlobal->MaxAuthVariableSize - sizeof (AUTH_VAR_HEADER);
  Status = AuthVariableLibInitialize (&mAuthContextIn, &mAuthContextOut);
  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Variable driver will work with auth variable support!\n"));
    if (mAuthContextOut.AuthVarEntry != NULL) {
      for (Index = 0; Index < mAuthContextOut.AuthVarEntryCount; Index++) {
        VariableEntry = &mAuthContextOut.AuthVarEntry[Index];
        Status = VarCheckLibVariablePropertySet (
                   VariableEntry->Name,
                   VariableEntry->Guid,
                   &VariableEntry->VariableProperty
                   );
        ASSERT_EFI_ERROR (Status);
      }
    }
  } else {
    DEBUG ((EFI_D_ERROR, "Error: AuthVariableLibInitialize() returns %r!\n", Status));
    return Status;
  }

  for (Index = 0; Index < sizeof (mVariableEntryProperty) / sizeof (mVariableEntryProperty[0]);
       Index++) {
    VariableEntry = &mVariableEntryProperty[Index];
    Status = VarCheckLibVariablePropertySet (
                   VariableEntry->Name,
                   VariableEntry->Guid,
                   &VariableEntry->VariableProperty
                   );
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}


#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
VOID
SimulateOutage (
  UINT32  Flag
  )
{
  DEBUG ((EFI_D_ERROR, "SimulateOutage %d!\n", Flag));
}
#endif


/**
  Update the variable region with Variable information. These are the same 
  arguments as the EFI Variable services.

  @param[in] VariableName       Name of variable

  @param[in] VendorGuid         Guid of variable

  @param[in] Data               Variable data

  @param[in] DataSize           Size of data. 0 means delete

  @param[in] Attributes         Attribues of the variable

  @param[in] KeyIndex           Index of associated public key.

  @param[in] MonotonicCount     Value of associated monotonic count.

  @param[in] Variable           The variable information which is used to keep track of variable usage.

  @param[in] TimeStamp          Value of associated TimeStamp.

  @param[in] DidReclaim         Whether Reclaim() has already been tried.  Normal calls to this function
                                are expected to set this parameter to FALSE.  This parameter is set to
                                TRUE when this function calls itself after a Reclaim() has already been
                                tried.

  @retval EFI_SUCCESS           The update operation is success.

  @retval EFI_OUT_OF_RESOURCES  Variable region is full, can not write other data into this region.

**/
EFI_STATUS
EFIAPI
UpdateVariable (
  IN      CHAR16                  *VariableName,
  IN      EFI_GUID                *VendorGuid,
  IN      VOID                    *Data,
  IN      UINTN                   DataSize,
  IN      UINT32                  Attributes,
  IN      UINT32                  KeyIndex        OPTIONAL,
  IN      UINT64                  MonotonicCount  OPTIONAL,
  IN      VARIABLE_POINTER_TRACK  *Variable,
  IN      EFI_TIME                *TimeStamp      OPTIONAL,
  IN      BOOLEAN                 DidReclaim
  )
{
  EFI_STATUS              Status;
#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
  UINT32                  OutageFlag;
#endif
  VARIABLE_MODULE_GLOBAL  *EVG;
  VARIABLE_GLOBAL         *Global;
  AUTH_VAR_HEADER         *NextVariable;
  UINTN                   MaxDataSize;
  UINTN                   VarNameSize;
  UINTN                   VarNameOffset;
  UINTN                   VarDataOffset;
  UINTN                   VarSize;
  UINTN                   NonVolatileVarableStoreSize;
  UINTN                   DataOffset;
  UINTN                   OrigDataSize;
  BOOLEAN                 DeleteUpdating;

#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
  OutageFlag = (Attributes & EFI_VARIABLE_OUTAGES_MASK) >> EFI_VARIABLE_OUTAGES_SHIFT;
#endif

  OrigDataSize = 0;

  EVG = mVariableModuleGlobal;
  Global = &EVG->VariableGlobal;
#define NVFLUSH(Pointer, Size) \
    FlushToNvStore ((EFI_PHYSICAL_ADDRESS)(Pointer) - Global->NonVolatileVariableBase, \
                    (UINT8 *)(Pointer), Size)

  if (Variable->CurrPtr != NULL) {
    //
    // Update/Delete existing variable
    //

    if (EfiAtRuntime ()) {        
      //
      // If EfiAtRuntime and the variable is Volatile and Runtime Access,  
      // the volatile is ReadOnly, and SetVariable should be aborted and 
      // return EFI_WRITE_PROTECTED.
      //
      if (Variable->Volatile) {
        Status = EFI_WRITE_PROTECTED;
        goto Done;
      }
      //
      // Only variable have NV attribute can be updated/deleted in Runtime
      //
      if ((Variable->CurrPtr->Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }

    //
    // Setting a data variable with no access, or zero DataSize attributes
    // specified causes it to be deleted.
    //
    // When the EFI_VARIABLE_APPEND_WRITE attribute is set, DataSize
    // of zero will not delete the variable.
    //
    if ((((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) && DataSize == 0) ||
        (Attributes & VARIABLE_ATTRIBUTE_BS_RT) == 0) {
      if (Variable->InDeletedTransitionPtr != NULL) {
        //
        // Both ADDED and IN_DELETED_TRANSITION variable are present,
        // set IN_DELETED_TRANSITION one to DELETED state first.
        //
        Variable->InDeletedTransitionPtr->State &= VAR_DELETED;
        if (IsActiveNvStore () && !Variable->Volatile) {
          NVFLUSH (&Variable->InDeletedTransitionPtr->State,
                   sizeof (Variable->InDeletedTransitionPtr->State));
        }
      }
      //
      // Mark the old variable as deleted
      //
      Variable->CurrPtr->State &= VAR_DELETED;
      if (IsActiveNvStore () && !Variable->Volatile) {
        NVFLUSH (&Variable->CurrPtr->State,
                 sizeof (Variable->CurrPtr->State));
      }
      UpdateVariableInfo (VariableName, VendorGuid, Variable->Volatile, FALSE, FALSE, TRUE, FALSE);
      Status = EFI_SUCCESS;
      goto Done;
    }

    //
    // If the variable is marked valid and the same data has been passed in
    // then return to the caller immediately.
    //
    if (Variable->CurrPtr->DataSize == DataSize &&
        CompareMem (Data, GetVariableDataPtr (Variable->CurrPtr), DataSize) == 0 &&
        (Attributes & EFI_VARIABLE_APPEND_WRITE) == 0 &&
        TimeStamp == NULL
        ) {
      Status = EFI_SUCCESS;
      goto Done;
    }

    //
    // EFI_VARIABLE_APPEND_WRITE attribute only affects existing variables.
    //
    if ((Attributes & EFI_VARIABLE_APPEND_WRITE) != 0) {
      //
      // Set Max Common/Auth Variable Data Size as default MaxDataSize.
      //
      DataOffset = GetVariableDataOffset (Variable->CurrPtr);
      if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
        MaxDataSize = PcdGet32 (PcdMaxHardwareErrorVariableSize) - DataOffset;
      } else if ((Attributes & VARIABLE_ATTRIBUTE_AT_AW) != 0) {
        MaxDataSize = EVG->MaxAuthVariableSize - DataOffset;
      } else {
        MaxDataSize = EVG->MaxVariableSize - DataOffset;
      }

      //
      // Verify that there is room to append the new data.
      //
      OrigDataSize = Variable->CurrPtr->DataSize;
      if (OrigDataSize + DataSize > MaxDataSize) {
        //
        // Existing data size + new data size exceed maximum
        // variable size limitation.
        //
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }

  } else {
    //
    // Handle the case where no existing variable is found, so we need
    // to create a new variable.
    //  
    if ((DataSize == 0) && ((Attributes & EFI_VARIABLE_APPEND_WRITE) != 0)) {
      Status = EFI_SUCCESS;
        goto Done;
    }
    
    //
    // Make sure we are trying to create a new variable.
    // Setting a data variable with no access, or zero DataSize
    // attributes means to delete it.
    //
    if (DataSize == 0 || (Attributes & VARIABLE_ATTRIBUTE_BS_RT) == 0) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
        
    //
    // Only variable have NV|RT attribute can be created in Runtime
    //
    if (EfiAtRuntime () &&
        (((Attributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0) ||
         ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0))) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }         
  }
  
  //
  // Function part - create a new variable and copy the data.
  // Both update a variable and create a variable will come here.
  //

  VarNameOffset = sizeof (AUTH_VAR_HEADER);
  VarNameSize   = StrSize (VariableName);
  VarDataOffset = VarNameOffset + VarNameSize + GET_PAD_SIZE (VarNameSize);
  VarSize       = (VarDataOffset + OrigDataSize + DataSize +
                   GET_PAD_SIZE (OrigDataSize + DataSize));

  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0) {
    NonVolatileVarableStoreSize = ((VARIABLE_STORE_HEADER *)(Global->NonVolatileVariableBase))->Size;
    if ((((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) &&
         ((HEADER_ALIGN (VarSize) + EVG->HwErrVariableTotalSize) >
          PcdGet32 (PcdHwErrStorageSize))) ||
        (((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == 0) &&
         ((HEADER_ALIGN (VarSize) + EVG->CommonVariableTotalSize) >
          NonVolatileVarableStoreSize - sizeof (VARIABLE_STORE_HEADER) - PcdGet32 (PcdHwErrStorageSize)))) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    NextVariable = (AUTH_VAR_HEADER *)(EVG->NonVolatileLastVariableOffset
                      + (UINTN) Global->NonVolatileVariableBase);
    EVG->NonVolatileLastVariableOffset += HEADER_ALIGN (VarSize);

    if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
      EVG->HwErrVariableTotalSize += HEADER_ALIGN (VarSize);
    } else {
      EVG->CommonVariableTotalSize += HEADER_ALIGN (VarSize);
    }
  } else {
    if ((UINT32) (HEADER_ALIGN (VarSize) + EVG->VolatileLastVariableOffset) >
          ((VARIABLE_STORE_HEADER *) ((UINTN) (Global->VolatileVariableBase)))->Size
          ) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    NextVariable    = (AUTH_VAR_HEADER *)(EVG->VolatileLastVariableOffset
                        + (UINTN) Global->VolatileVariableBase);
    EVG->VolatileLastVariableOffset += HEADER_ALIGN (VarSize);
  }

  NextVariable->StartId        = VARIABLE_DATA;
  NextVariable->Attributes     = Attributes & ~EFI_VARIABLE_APPEND_WRITE;
#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
  NextVariable->Attributes    &= ~EFI_VARIABLE_OUTAGES_MASK;
#endif
  NextVariable->State          = VAR_ADDED;
  NextVariable->Reserved       = 0;
  NextVariable->MonotonicCount = 0;
  NextVariable->PubKeyIndex    = 0;
  ZeroMem (&NextVariable->TimeStamp, sizeof (EFI_TIME));

  if ((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) != 0 &&
      TimeStamp != NULL) {
    if ((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) {
      CopyMem (&NextVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
    } else {
      //
      // In the case when the EFI_VARIABLE_APPEND_WRITE attribute is set, only
      // when the new TimeStamp value is later than the current timestamp associated
      // with the variable, we need associate the new timestamp with the updated value.
      //
      if (Variable->CurrPtr != NULL) {
        if (VariableCompareTimeStampInternal (&Variable->CurrPtr->TimeStamp, TimeStamp)) {
          CopyMem (&NextVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
        }
      }
    }
  }

  //
  // There will be pad bytes after Data, the NextVariable->NameSize and
  // NextVariable->DataSize should not include pad size so that variable
  // service can get actual size in GetVariable
  //
  NextVariable->NameSize = VarNameSize;
  NextVariable->DataSize = OrigDataSize + DataSize;

  CopyMem (&NextVariable->VendorGuid, VendorGuid, sizeof (EFI_GUID));
  CopyMem (
    (UINT8 *) ((UINTN) NextVariable + VarNameOffset),
    VariableName,
    VarNameSize
    );

  if (OrigDataSize == 0) {
    CopyMem (
      (UINT8 *)NextVariable + VarDataOffset,
      Data,
      DataSize
      );
  } else {
    CopyMem (
      (UINT8 *)NextVariable + VarDataOffset,
      (UINT8 *)Variable->CurrPtr + VarDataOffset,
      OrigDataSize
      );
    CopyMem (
      (UINT8 *)NextVariable + VarDataOffset + OrigDataSize,
      Data,
      DataSize
      );
  }

  //
  // Mark the old variable as in deleted transition
  //
  if (Variable->CurrPtr != NULL) {
    Variable->CurrPtr->State &= VAR_IN_DELETED_TRANSITION;
    if (IsActiveNvStore () && (Attributes & EFI_VARIABLE_NON_VOLATILE)) {
      NVFLUSH (&Variable->CurrPtr->State, sizeof (Variable->CurrPtr->State));
#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
      if (OutageFlag == 1) {
        SimulateOutage (OutageFlag);
        return EFI_SUCCESS;
      }
#endif
    }
  }

  // Update NvStore.
  if (IsActiveNvStore () && (Attributes & EFI_VARIABLE_NON_VOLATILE)) {
    //
    // Temporarily give the new variable an invalid StardId, and copy
    // it to NvStore.
    //
    NextVariable->StartId = 0;
    NVFLUSH (NextVariable, VarSize);
#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
    if (OutageFlag == 2) {
      SimulateOutage (OutageFlag);
      return EFI_SUCCESS;
    }
#endif

    //
    // Set and copy the StartId of the new variable.
    //
    NextVariable->StartId = VARIABLE_DATA;
    NVFLUSH (&NextVariable->StartId, sizeof (NextVariable->StartId));
#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
    if (OutageFlag == 3) {
      SimulateOutage (OutageFlag);
      return EFI_SUCCESS;
    }
#endif

    //
    // Copy the state of the old variable (VAR_DELETED) to NvStore.
    //
    if (Variable->CurrPtr != NULL) {
      Variable->CurrPtr->State &= VAR_DELETED;
      NVFLUSH (&Variable->CurrPtr->State, sizeof (Variable->CurrPtr->State));
    }
  } else {
    //
    // Mark the old variable as deleted
    //
    if (Variable->CurrPtr != NULL) {
      Variable->CurrPtr->State &= VAR_DELETED;
    }
  }

  UpdateVariableInfo (VariableName, VendorGuid, Variable->Volatile, FALSE, TRUE, FALSE, FALSE);

  Status = EFI_SUCCESS;

Done:
  if (Status == EFI_OUT_OF_RESOURCES && !DidReclaim) {
    DeleteUpdating = ((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0);

    if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0) {
      Status = Reclaim (Global->NonVolatileVariableBase,
                        &mVariableModuleGlobal->NonVolatileLastVariableOffset,
                        FALSE, Variable, DeleteUpdating);
    } else {
      Status = Reclaim (Global->VolatileVariableBase,
                        &mVariableModuleGlobal->VolatileLastVariableOffset,
                        TRUE, Variable, DeleteUpdating);
    }
    ASSERT_EFI_ERROR (Status);

    return UpdateVariable (VariableName, VendorGuid, Data, DataSize, Attributes,
                           KeyIndex, MonotonicCount, Variable, TimeStamp, TRUE);
  }

  return Status;

#undef NVFLUSH
}

/**
  Find the variable in the variable store region described by PtrTrack.

  @param[in]       VariableName        Name of the variable to be found
  @param[in]       VendorGuid          Vendor GUID to be found.
  @param[in]       IgnoreRtCheck       Ignore EFI_VARIABLE_RUNTIME_ACCESS attribute
                                       check at runtime when searching variable.
  @param[in, out]  PtrTrack            Variable Track Pointer structure that contains Variable Information.

  @retval          EFI_SUCCESS         Variable found successfully
  @retval          EFI_NOT_FOUND       Variable not found
**/
EFI_STATUS
FindVariableEx (
  IN     CHAR16                  *VariableName,
  IN     EFI_GUID                *VendorGuid,
  IN     BOOLEAN                 IgnoreRtCheck,
  IN OUT VARIABLE_POINTER_TRACK  *PtrTrack
  )
{
  AUTH_VAR_HEADER  *InDeletedVariable;

  PtrTrack->InDeletedTransitionPtr = NULL;
  InDeletedVariable  = NULL;

  for (PtrTrack->CurrPtr = PtrTrack->StartPtr;
       IsValidVariableHeader (PtrTrack->CurrPtr, PtrTrack->EndPtr);
       PtrTrack->CurrPtr = GetNextVariablePtr (PtrTrack->CurrPtr)
      ) {
    if (VariableIsLive (PtrTrack->CurrPtr)) {
      if (IgnoreRtCheck || !EfiAtRuntime () ||
          ((PtrTrack->CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0)) {
        if (VariableName[0] == 0 ||
            (CompareGuid (VendorGuid, &PtrTrack->CurrPtr->VendorGuid) &&
             CompareMem (VariableName, GetVariableNamePtr (PtrTrack->CurrPtr),
                         PtrTrack->CurrPtr->NameSize) == 0)) {
          if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
            InDeletedVariable = PtrTrack->CurrPtr;
          } else {
            PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
            return EFI_SUCCESS;
          }
        }
      }
    }
  }

  PtrTrack->CurrPtr = InDeletedVariable;
  return (PtrTrack->CurrPtr  == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
}

/**
  Finds variable in storage blocks of volatile and non-volatile storage areas.

  This code finds variable in storage blocks of volatile and non-volatile storage areas.
  If VariableName is an empty string, then we just return the first
  qualified variable without comparing VariableName and VendorGuid.
  Otherwise, VariableName and VendorGuid are compared.

  @param  VariableName                Name of the variable to be found.
  @param  VendorGuid                  Vendor GUID to be found.
  @param  PtrTrack                    VARIABLE_POINTER_TRACK structure for output,
                                      including the range searched and the target position.
  @param  Global                      Pointer to VARIABLE_GLOBAL structure, including
                                      base of volatile variable storage area, base of
                                      NV variable storage area, and a lock.

  @retval EFI_INVALID_PARAMETER       If VariableName is not an empty string, while
                                      VendorGuid is NULL.
  @retval EFI_SUCCESS                 Variable successfully found.
  @retval EFI_NOT_FOUND               Variable not found.

**/
EFI_STATUS
FindVariable (
  IN  CHAR16                  *VariableName,
  IN  EFI_GUID                *VendorGuid,
  OUT VARIABLE_POINTER_TRACK  *PtrTrack,
  IN  VARIABLE_GLOBAL         *Global
  )
{
  EFI_STATUS             Status;
  UINTN                  Index;
  VARIABLE_STORE_HEADER *Header[VariableStoreTypeMax];

  if (VariableName[0] != 0 && VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Header[VariableStoreTypeVolatile] = (VARIABLE_STORE_HEADER *)Global->VolatileVariableBase;
  Header[VariableStoreTypeNv]       = (VARIABLE_STORE_HEADER *)Global->NonVolatileVariableBase;

  //
  // Find the variable by walk through non-volatile and volatile variable store
  //
  for (Index = 0; Index < VariableStoreTypeMax; Index++) {
    PtrTrack->StartPtr  = GetStartPointer (Header[Index]);
    PtrTrack->EndPtr    = GetEndPointer   (Header[Index]);
    PtrTrack->Volatile  = (Index == VariableStoreTypeVolatile);

    Status = FindVariableEx (VariableName, VendorGuid, FALSE, PtrTrack);
    if (!EFI_ERROR (Status)) {
      return Status;
    }
  }
  return EFI_NOT_FOUND;
}

/**
  This function checks to see if the remaining variable space is enough to set
  all variables from the argument list successfully.

  Note: Variables are assumed to be in same storage.
  The set sequence of Variables will be same with the sequence of VariableEntry from argument list,
  so follow the argument sequence to check the Variables.

  @param[in] Attributes         Variable attributes for Variable entries.
  @param[in] Marker             VA_LIST style variable argument list.
                                The variable argument list with type VARIABLE_ENTRY_CONSISTENCY *.
                                A NULL terminates the list. The VariableSize of
                                VARIABLE_ENTRY_CONSISTENCY is the variable data size as input.
                                It will be changed to variable total size as output.

  @retval TRUE                  Have enough variable space to set the Variables successfully.
  @retval FALSE                 Not enough variable space to set the Variables successfully.

**/
BOOLEAN
EFIAPI
CheckRemainingSpaceForConsistencyInternal (
  IN UINT32                     Attributes,
  IN VA_LIST                    Marker
  )
{
  EFI_STATUS                    Status;
  VA_LIST                       Args;
  VARIABLE_ENTRY_CONSISTENCY    *VariableEntry;
  UINT64                        MaximumVariableStorageSize;
  UINT64                        RemainingVariableStorageSize;
  UINT64                        MaximumVariableSize;
  UINTN                         TotalNeededSize;
  UINTN                         OriginalVarSize;
  VARIABLE_STORE_HEADER         *VariableStoreHeader;
  VARIABLE_POINTER_TRACK        VariablePtrTrack;
  AUTH_VAR_HEADER               *NextVariable;
  UINTN                         VarNameSize;
  UINTN                         VarDataSize;

  Status = EmuQueryVariableInfoInternal (
             Attributes,
             &MaximumVariableStorageSize,
             &RemainingVariableStorageSize,
             &MaximumVariableSize,
             &mVariableModuleGlobal->VariableGlobal
             );
  ASSERT_EFI_ERROR (Status);

  TotalNeededSize = 0;
  Args = Marker;
  VariableEntry = VA_ARG (Args, VARIABLE_ENTRY_CONSISTENCY *);
  while (VariableEntry != NULL) {
    //
    // Calculate variable total size.
    //
    VarNameSize  = StrSize (VariableEntry->Name);
    VarNameSize += GET_PAD_SIZE (VarNameSize);
    VarDataSize  = VariableEntry->VariableSize;
    VarDataSize += GET_PAD_SIZE (VarDataSize);
    VariableEntry->VariableSize = HEADER_ALIGN (sizeof (AUTH_VAR_HEADER) + VarNameSize + VarDataSize);

    TotalNeededSize += VariableEntry->VariableSize;
    VariableEntry = VA_ARG (Args, VARIABLE_ENTRY_CONSISTENCY *);
  }

  if (RemainingVariableStorageSize >= TotalNeededSize) {
    //
    // Already have enough space.
    //
    return TRUE;
  } else if (EfiAtRuntime ()) {
    //
    // At runtime, no reclaim.
    // The original variable space of Variables can't be reused.
    //
    return FALSE;
  }

  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    VariableStoreHeader = (VARIABLE_STORE_HEADER *)
      mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  } else {
    VariableStoreHeader = (VARIABLE_STORE_HEADER *)
      mVariableModuleGlobal->VariableGlobal.VolatileVariableBase;
  }

  Args = Marker;
  VariableEntry = VA_ARG (Args, VARIABLE_ENTRY_CONSISTENCY *);
  while (VariableEntry != NULL) {
    //
    // Check if the variable described by VariableEntry exists, and
    // get its size.
    //
    OriginalVarSize = 0;
    VariablePtrTrack.StartPtr = GetStartPointer (VariableStoreHeader);
    VariablePtrTrack.EndPtr   = GetEndPointer   (VariableStoreHeader);
    Status = FindVariableEx (
               VariableEntry->Name,
               VariableEntry->Guid,
               FALSE,
               &VariablePtrTrack
               );
    if (!EFI_ERROR (Status)) {
      //
      // Get size of the variable.
      //
      NextVariable = GetNextVariablePtr (VariablePtrTrack.CurrPtr);
      OriginalVarSize = (UINTN) NextVariable - (UINTN) VariablePtrTrack.CurrPtr;
      //
      // Add the original size of the variable to remaining variable storage size.
      //
      RemainingVariableStorageSize += OriginalVarSize;
    }
    if (VariableEntry->VariableSize > RemainingVariableStorageSize) {
      //
      // Not enough space for the variable.
      //
      return FALSE;
    }
    //
    // Subtract the (new) size of the variable from remaining variable storage size.
    //
    RemainingVariableStorageSize -= VariableEntry->VariableSize;
    VariableEntry = VA_ARG (Args, VARIABLE_ENTRY_CONSISTENCY *);
  }

  return TRUE;
}

/**
  This code finds variable in storage blocks (Volatile or Non-Volatile).

  @param  VariableName           A Null-terminated Unicode string that is the name of
                                 the vendor's variable.
  @param  VendorGuid             A unique identifier for the vendor.
  @param  Attributes             If not NULL, a pointer to the memory location to return the
                                 attributes bitmask for the variable.
  @param  DataSize               Size of Data found. If size is less than the
                                 data, this value contains the required size.
  @param  Data                   On input, the size in bytes of the return Data buffer.
                                 On output, the size of data returned in Data.
  @param  Global                 Pointer to VARIABLE_GLOBAL structure

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_NOT_FOUND          The variable was not found.
  @retval EFI_BUFFER_TOO_SMALL   DataSize is too small for the result.  DataSize has
                                 been updated with the size needed to complete the request.
  @retval EFI_INVALID_PARAMETER  VariableName or VendorGuid or DataSize is NULL.

**/
EFI_STATUS
EFIAPI
EmuGetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data,
  IN      VARIABLE_GLOBAL   *Global
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  UINTN                   VarDataSize;
  EFI_STATUS              Status;
  UINT8                   *VariableDataPtr;

  if (VariableName == NULL || VendorGuid == NULL || DataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AcquireLockOnlyAtBootTime(&Global->VariableServicesLock);

  InitNvStore ();

  Status = InitAuthenticatedVariables ();
  ASSERT_EFI_ERROR (Status);

  //
  // Find existing variable
  //
  Status = FindVariable (VariableName, VendorGuid, &Variable, Global);

  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Get data size
  //
  VarDataSize = Variable.CurrPtr->DataSize;
  if (*DataSize >= VarDataSize) {
    if (Data == NULL) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    VariableDataPtr = GetVariableDataPtr (Variable.CurrPtr);
    ASSERT (VariableDataPtr != NULL);
    
    CopyMem (Data, VariableDataPtr, VarDataSize);
    if (Attributes != NULL) {
      *Attributes = Variable.CurrPtr->Attributes;
    }

    *DataSize = VarDataSize;
    UpdateVariableInfo (VariableName, VendorGuid, Variable.Volatile, TRUE, FALSE, FALSE, FALSE);
    Status = EFI_SUCCESS;
    goto Done;
  } else {
    *DataSize = VarDataSize;
    Status = EFI_BUFFER_TOO_SMALL;
    goto Done;
  }

Done:
  ReleaseLockOnlyAtBootTime (&Global->VariableServicesLock);
  return Status;
}

/**
  This code finds the next available variable after a given variable.

  If there is a variable after the given variable in the current
  store, that variable is returned.  Otherwise, the first variable in
  the next store is returned.

  If the given name is "", then the first valid variable is returned.

  Caution: This function may receive untrusted input.

  @param[in]  VariableName  Pointer to variable name.
  @param[in]  VendorGuid    Variable Vendor Guid.
  @param[out] VariablePtr   Pointer to variable header address.

  @return EFI_SUCCESS       Find the specified variable.
  @return EFI_NOT_FOUND     Not found.

**/
EFI_STATUS
EFIAPI
EmuGetNextVariableInternal (
  IN  CHAR16                *VariableName,
  IN  EFI_GUID              *VendorGuid,
  OUT AUTH_VAR_HEADER      **VariablePtr,
  IN  VARIABLE_GLOBAL       *Global
  )
{
  EFI_STATUS              Status;
  VARIABLE_STORE_TYPE     Type;
  VARIABLE_POINTER_TRACK  Variable;
  VARIABLE_POINTER_TRACK  VariablePtrTrack;
  VARIABLE_STORE_HEADER   *VariableStoreHeader[VariableStoreTypeMax];

  Status = FindVariable (VariableName, VendorGuid, &Variable, Global);
  if (Variable.CurrPtr == NULL || EFI_ERROR (Status)) {
    goto Done;
  }

  if (VariableName[0] != 0) {
    //
    // If variable name is not NULL, get next variable.
    //
    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

  //
  // The index and attributes mapping must be kept in this order as
  // FindVariable makes use of this mapping to implement search
  // algorithm.
  //
  VariableStoreHeader[VariableStoreTypeVolatile] = (VARIABLE_STORE_HEADER *)Global->VolatileVariableBase;
  VariableStoreHeader[VariableStoreTypeNv]       = (VARIABLE_STORE_HEADER *)Global->NonVolatileVariableBase;

  while (TRUE) {
    //
    // Iterate through each store type.
    //
    while (!IsValidVariableHeader (Variable.CurrPtr, Variable.EndPtr)) {
      //
      // Find current storage index
      //
      for (Type = (VARIABLE_STORE_TYPE) 0; Type < VariableStoreTypeMax; Type++) {
        if ((VariableStoreHeader[Type] != NULL) &&
            (Variable.StartPtr == GetStartPointer (VariableStoreHeader[Type]))) {
          break;
        }
      }
      ASSERT (Type < VariableStoreTypeMax);
      //
      // Switch to next storage
      //
      for (Type++; Type < VariableStoreTypeMax; Type++) {
        if (VariableStoreHeader[Type] != NULL) {
          break;
        }
      }
      //
      // Capture the case that
      // 1. current storage is the last one, or
      // 2. no further storage
      //
      if (Type >= VariableStoreTypeMax) {
        Status = EFI_NOT_FOUND;
        goto Done;
      }
      Variable.StartPtr = GetStartPointer (VariableStoreHeader[Type]);
      Variable.EndPtr   = GetEndPointer   (VariableStoreHeader[Type]);
      Variable.CurrPtr  = Variable.StartPtr;
    }

    //
    // Variable is found
    //
    if (VariableIsLive (Variable.CurrPtr)) {
      if (!EfiAtRuntime () ||
          ((Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0)) {
        if (Variable.CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
          //
          // If it is a IN_DELETED_TRANSITION variable,
          // and there is also a same ADDED one at the same time,
          // don't return it.
          //
          VariablePtrTrack.StartPtr = Variable.StartPtr;
          VariablePtrTrack.EndPtr = Variable.EndPtr;
          Status = FindVariableEx (
                     GetVariableNamePtr (Variable.CurrPtr),
                     &Variable.CurrPtr->VendorGuid,
                     FALSE,
                     &VariablePtrTrack
                     );
          if (!EFI_ERROR (Status) && VariablePtrTrack.CurrPtr->State == VAR_ADDED) {
            Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
            continue;
          }
        }

        *VariablePtr = Variable.CurrPtr;
        Status = EFI_SUCCESS;
        goto Done;
      }
    }

    Variable.CurrPtr = GetNextVariablePtr (Variable.CurrPtr);
  }

Done:
  return Status;
}

/**

  This code Finds the Next available variable.

  @param  VariableNameSize       Size of the variable.
  @param  VariableName           On input, supplies the last VariableName that was returned by GetNextVariableName().
                                 On output, returns the Null-terminated Unicode string of the current variable.
  @param  VendorGuid             On input, supplies the last VendorGuid that was returned by GetNextVariableName().
                                 On output, returns the VendorGuid of the current variable.  
  @param  Global                 Pointer to VARIABLE_GLOBAL structure.

  @retval EFI_SUCCESS            The function completed successfully. 
  @retval EFI_NOT_FOUND          The next variable was not found.
  @retval EFI_BUFFER_TOO_SMALL   VariableNameSize is too small for the result. 
                                 VariableNameSize has been updated with the size needed to complete the request.
  @retval EFI_INVALID_PARAMETER  VariableNameSize or VariableName or VendorGuid is NULL.

**/
EFI_STATUS
EFIAPI
EmuGetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid,
  IN      VARIABLE_GLOBAL   *Global
  )
{
  UINTN                   VarNameSize;
  EFI_STATUS              Status;
  AUTH_VAR_HEADER         *VariablePtr;

  if (VariableNameSize == NULL || VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AcquireLockOnlyAtBootTime(&Global->VariableServicesLock);

  Status = EmuGetNextVariableInternal (VariableName, VendorGuid, &VariablePtr, Global);
  if (!EFI_ERROR (Status)) {
    VarNameSize = VariablePtr->NameSize;
    ASSERT (VarNameSize != 0);
    if (VarNameSize <= *VariableNameSize) {
      CopyMem (VariableName, GetVariableNamePtr (VariablePtr), VarNameSize);
      CopyMem (VendorGuid, &VariablePtr->VendorGuid, sizeof (EFI_GUID));
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_BUFFER_TOO_SMALL;
    }

    *VariableNameSize = VarNameSize;
  }

  ReleaseLockOnlyAtBootTime (&Global->VariableServicesLock);
  return Status;
}

/**

  This code sets variable in storage blocks (Volatile or Non-Volatile).

  @param  VariableName           A Null-terminated Unicode string that is the name of the vendor's
                                 variable.  Each VariableName is unique for each 
                                 VendorGuid.  VariableName must contain 1 or more 
                                 Unicode characters.  If VariableName is an empty Unicode 
                                 string, then EFI_INVALID_PARAMETER is returned.
  @param  VendorGuid             A unique identifier for the vendor
  @param  Attributes             Attributes bitmask to set for the variable
  @param  DataSize               The size in bytes of the Data buffer.  A size of zero causes the
                                 variable to be deleted.
  @param  Data                   The contents for the variable
  @param  Global                 Pointer to VARIABLE_GLOBAL structure
  @param  VolatileOffset         The offset of last volatile variable
  @param  NonVolatileOffset      The offset of last non-volatile variable

  @retval EFI_SUCCESS            The firmware has successfully stored the variable and its data as 
                                 defined by the Attributes.
  @retval EFI_INVALID_PARAMETER  An invalid combination of attribute bits was supplied, or the 
                                 DataSize exceeds the maximum allowed, or VariableName is an empty 
                                 Unicode string, or VendorGuid is NULL.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved due to a hardware failure.
  @retval EFI_WRITE_PROTECTED    The variable in question is read-only or cannot be deleted.
  @retval EFI_NOT_FOUND          The variable trying to be updated or deleted was not found.

**/
EFI_STATUS
EFIAPI
EmuSetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data,
  IN VARIABLE_GLOBAL         *Global,
  IN UINTN                   *VolatileOffset,
  IN UINTN                   *NonVolatileOffset
  )
{
  VARIABLE_POINTER_TRACK  Variable;
  EFI_STATUS              Status;
  UINTN                   PayloadSize;

  //
  // Check input parameters
  //
  if (VariableName == NULL || VariableName[0] == 0 || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }  

  if (DataSize != 0 && Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check for reserved bits in variable attribute.
  //
  if ((Attributes & ~EFI_VARIABLE_ATTRIBUTES_MASK) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Make sure if runtime bit is set, boot service bit is set also
  //
  if ((Attributes & VARIABLE_ATTRIBUTE_BS_RT) == EFI_VARIABLE_RUNTIME_ACCESS) {
    return EFI_INVALID_PARAMETER;
  } else if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
    if (PcdGet32 (PcdHwErrStorageSize) == 0) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS and
  // EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS attribute
  // cannot both be set.
  //
  if (((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) != 0) &&
      ((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS) != 0) {
    if (DataSize < AUTHINFO_SIZE) {
      //
      // Try to write Authenticated Variable without AuthInfo.
      //
      return EFI_SECURITY_VIOLATION;
    }
    PayloadSize = DataSize - AUTHINFO_SIZE;
  } else if ((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) != 0) {
    //
    // Sanity check for EFI_VARIABLE_AUTHENTICATION_2 descriptor.
    //
    if (DataSize < OFFSET_OF_AUTHINFO2_CERT_DATA ||
        ((EFI_VARIABLE_AUTHENTICATION_2 *) Data)->AuthInfo.Hdr.dwLength >
        DataSize - (OFFSET_OF (EFI_VARIABLE_AUTHENTICATION_2, AuthInfo)) ||
        ((EFI_VARIABLE_AUTHENTICATION_2 *) Data)->AuthInfo.Hdr.dwLength <
        OFFSET_OF (WIN_CERTIFICATE_UEFI_GUID, CertData)) {
      return EFI_SECURITY_VIOLATION;
    }
    PayloadSize = DataSize - AUTHINFO2_SIZE (Data);
  } else {
    PayloadSize = DataSize;
  }
  
  if ((UINTN)(~0) - PayloadSize < StrSize(VariableName)){
    //
    // Prevent whole variable size overflow 
    // 
    return EFI_INVALID_PARAMETER;
  }

  //
  //  The size of the VariableName, including the Unicode Null in
  //  bytes plus the DataSize is limited to maximum size of PcdGet32
  //  (PcdMaxHardwareErrorVariableSize) bytes for HwErrRec, and
  //  PcdGet32 (PcdMaxVariableSize) bytes for the others.
  //
  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
    if (StrSize (VariableName) + PayloadSize >
        (PcdGet32 (PcdMaxHardwareErrorVariableSize) - sizeof (AUTH_VAR_HEADER))) {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    //  The size of the VariableName, including the Unicode Null in bytes plus
    //  the PayloadSize is limited to maximum size of Max(Auth)VariableSize
    //  bytes.
    //
    if ((Attributes & VARIABLE_ATTRIBUTE_AT_AW) != 0) {
      if (StrSize (VariableName) + PayloadSize >
          mVariableModuleGlobal->MaxAuthVariableSize - sizeof (AUTH_VAR_HEADER)) {
        return EFI_INVALID_PARAMETER;
      }
    } else {
      if (StrSize (VariableName) + PayloadSize >
          mVariableModuleGlobal->MaxVariableSize - sizeof (AUTH_VAR_HEADER)) {
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  Status = VarCheckLibSetVariableCheck (
    VariableName, VendorGuid, Attributes, PayloadSize,
    (VOID *) ((UINTN) Data + DataSize - PayloadSize), VarCheckFromUntrusted);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AcquireLockOnlyAtBootTime(&Global->VariableServicesLock);

  InitNvStore ();

  InitAuthenticatedVariables ();

  //
  // Check whether the input variable is already existed
  //
  
  Status = FindVariable (VariableName, VendorGuid, &Variable, Global);
  if (!EFI_ERROR (Status)) {
    if (((Variable.CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0) &&
        EfiAtRuntime ()) {
      Status = EFI_WRITE_PROTECTED;
      goto Done;
    }
    if (Attributes != 0 &&
        (Attributes & (~EFI_VARIABLE_APPEND_WRITE)
#ifdef EFI_VARIABLE_SIMULATE_OUTAGE
         & (~EFI_VARIABLE_OUTAGES_MASK)
#endif
         ) != Variable.CurrPtr->Attributes) {
      //
      // If a preexisting variable is rewritten with different
      // attributes, SetVariable() shall not modify the variable and
      // shall return EFI_INVALID_PARAMETER. Two exceptions to this
      // rule:
      // 1. No access attributes specified
      // 2. The only attribute differing is EFI_VARIABLE_APPEND_WRITE
      //
      Status = EFI_INVALID_PARAMETER;
      DEBUG ((EFI_D_INFO, "[Variable]: Rewritten a preexisting variable(0x%08x) "
              "with different attributes(0x%08x) - %g:%s\n",
              Variable.CurrPtr->Attributes, Attributes, VendorGuid, VariableName));
      goto Done;
    }
  }

  //
  // Hook the operation of setting PlatformLangCodes/PlatformLang and LangCodes/Lang
  //
  AutoUpdateLangVariable (VariableName, Data, DataSize);

  Status = AuthVariableLibProcessVariable (VariableName, VendorGuid, Data, DataSize, Attributes);

Done:
  ReleaseLockOnlyAtBootTime (&Global->VariableServicesLock);
  return Status;
}

/**

  This code returns information about the EFI variables.

  @param  Attributes                   Attributes bitmask to specify the type of variables
                                       on which to return information.
  @param  MaximumVariableStorageSize   On output the maximum size of the storage space available for 
                                       the EFI variables associated with the attributes specified.  
  @param  RemainingVariableStorageSize Returns the remaining size of the storage space available for EFI 
                                       variables associated with the attributes specified.
  @param  MaximumVariableSize          Returns the maximum size of an individual EFI variable 
                                       associated with the attributes specified.
  @param  Global                       Pointer to VARIABLE_GLOBAL structure.

  @retval EFI_SUCCESS                  Valid answer returned.
  @retval EFI_INVALID_PARAMETER        An invalid combination of attribute bits was supplied
  @retval EFI_UNSUPPORTED              The attribute is not supported on this platform, and the 
                                       MaximumVariableStorageSize, RemainingVariableStorageSize, 
                                       MaximumVariableSize are undefined.

**/
EFI_STATUS
EFIAPI
EmuQueryVariableInfoInternal (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize,
  IN  VARIABLE_GLOBAL        *Global
  )
{
  AUTH_VAR_HEADER        *Variable;
  AUTH_VAR_HEADER        *NextVariable;
  UINT64                 VariableSize;
  VARIABLE_STORE_HEADER  *VariableStoreHeader;
  UINT64                 CommonVariableTotalSize;
  UINT64                 HwErrVariableTotalSize;

  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0) {
    //
    // Query is Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *)Global->VolatileVariableBase;
  } else {
    //
    // Query is Non-Volatile related.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *)Global->NonVolatileVariableBase;
  }

  //
  // Now let's fill *MaximumVariableStorageSize with the storage size
  // (excluding the storage header size)
  //
  *MaximumVariableStorageSize = VariableStoreHeader->Size - sizeof (VARIABLE_STORE_HEADER);

  //
  // Hardware error record variable needs larger size.
  //
  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0 &&
      (Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
    *MaximumVariableStorageSize = PcdGet32 (PcdHwErrStorageSize);
    *MaximumVariableSize = PcdGet32 (PcdMaxHardwareErrorVariableSize) - sizeof (AUTH_VAR_HEADER);
  } else {
    if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0) {
      ASSERT (PcdGet32 (PcdHwErrStorageSize) < VariableStoreHeader->Size);
      *MaximumVariableStorageSize = VariableStoreHeader->Size - sizeof (VARIABLE_STORE_HEADER) - PcdGet32 (PcdHwErrStorageSize);
    }

    //
    // Let *MaximumVariableSize be Max(Auth)VariableSize with the exception of the variable header size.
    //
    if ((Attributes & VARIABLE_ATTRIBUTE_AT_AW) != 0) {
      *MaximumVariableSize = mVariableModuleGlobal->MaxAuthVariableSize - sizeof (AUTH_VAR_HEADER);
    } else {
      *MaximumVariableSize = mVariableModuleGlobal->MaxVariableSize - sizeof (AUTH_VAR_HEADER);
    }
  }

  //
  // Point to the starting address of the variables.
  //
  Variable = GetStartPointer (VariableStoreHeader);

  //
  // Walk through the related variable store and compute the size
  // requirement.
  //
  CommonVariableTotalSize = 0;
  HwErrVariableTotalSize = 0;
  while (Variable < GetEndPointer (VariableStoreHeader)) {
    NextVariable = GetNextVariablePtr(Variable);
    if (NextVariable == NULL) {
      break;
    }
    VariableSize = (UINT64) NextVariable - (UINT64) Variable;

    if ((Variable->Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
      HwErrVariableTotalSize += VariableSize;
    } else {
      CommonVariableTotalSize += VariableSize;
    }

    //
    // Go to the next one.
    //
    Variable = NextVariable;
  }

  if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
    *RemainingVariableStorageSize = *MaximumVariableStorageSize - HwErrVariableTotalSize;
  } else {
    if (*MaximumVariableStorageSize < CommonVariableTotalSize) {
      *RemainingVariableStorageSize = 0;
    }
    else {
      *RemainingVariableStorageSize = *MaximumVariableStorageSize - CommonVariableTotalSize;
    }
  }

  if (*RemainingVariableStorageSize < sizeof (AUTH_VAR_HEADER)) {
    *MaximumVariableSize = 0;
  } else if ((*RemainingVariableStorageSize - sizeof (AUTH_VAR_HEADER)) < *MaximumVariableSize) {
    *MaximumVariableSize = *RemainingVariableStorageSize - sizeof (AUTH_VAR_HEADER);
  }

  return EFI_SUCCESS;
}

/**

  This code returns information about the EFI variables.

  @param  Attributes                   Attributes bitmask to specify the type of variables
                                       on which to return information.
  @param  MaximumVariableStorageSize   On output the maximum size of the storage space available for
                                       the EFI variables associated with the attributes specified.
  @param  RemainingVariableStorageSize Returns the remaining size of the storage space available for EFI
                                       variables associated with the attributes specified.
  @param  MaximumVariableSize          Returns the maximum size of an individual EFI variable
                                       associated with the attributes specified.
  @param  Global                       Pointer to VARIABLE_GLOBAL structure.

  @retval EFI_SUCCESS                  Valid answer returned.
  @retval EFI_INVALID_PARAMETER        An invalid combination of attribute bits was supplied
  @retval EFI_UNSUPPORTED              The attribute is not supported on this platform, and the
                                       MaximumVariableStorageSize, RemainingVariableStorageSize,
                                       MaximumVariableSize are undefined.

**/
EFI_STATUS
EFIAPI
EmuQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize,
  IN  VARIABLE_GLOBAL        *Global
  )
{
  EFI_STATUS             Status;

  if (MaximumVariableStorageSize == NULL || RemainingVariableStorageSize == NULL || MaximumVariableSize == NULL || Attributes == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Attributes & ~EFI_VARIABLE_ATTRIBUTES_MASK) == 0) {
    //
    // Make sure the Attributes combination is supported by the platform.
    //
    return EFI_UNSUPPORTED;
  } else if ((Attributes & VARIABLE_ATTRIBUTE_BS_RT) == EFI_VARIABLE_RUNTIME_ACCESS) {
    //
    // Make sure if runtime bit is set, boot service bit is set also.
    //
    return EFI_INVALID_PARAMETER;
  } else if (EfiAtRuntime () && ((Attributes & EFI_VARIABLE_RUNTIME_ACCESS) == 0)) {
    //
    //   Make sure RT Attribute is set if we are in Runtime phase.
    //
    return EFI_INVALID_PARAMETER;
  } else if ((Attributes & (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
    //
    // Make sure Hw Attribute is set with NV.
    //
    return EFI_INVALID_PARAMETER;
  }

  AcquireLockOnlyAtBootTime(&Global->VariableServicesLock);

  Status = EmuQueryVariableInfoInternal (
             Attributes,
             MaximumVariableStorageSize,
             RemainingVariableStorageSize,
             MaximumVariableSize,
             Global
             );

  ReleaseLockOnlyAtBootTime (&Global->VariableServicesLock);
  return Status;
}

/**
  Get non-volatile maximum variable size.

  @return Non-volatile maximum variable size.

**/
UINTN
GetNonVolatileMaxVariableSize (
  VOID
  )
{
  if (PcdGet32 (PcdHwErrStorageSize) != 0) {
    return MAX (MAX (PcdGet32 (PcdMaxVariableSize), PcdGet32 (PcdMaxAuthVariableSize)),
                PcdGet32 (PcdMaxHardwareErrorVariableSize));
  }

  return MAX (PcdGet32 (PcdMaxVariableSize), PcdGet32 (PcdMaxAuthVariableSize));
}

/**
  Initializes variable store area.

  This function allocates memory space for variable store area and initializes its attributes.

  @param  VolatileStore  Indicates if the variable store is volatile.

**/
EFI_STATUS
InitializeVariableStore (
  IN  BOOLEAN               VolatileStore
  )
{
  EFI_STATUS            Status;
  VARIABLE_STORE_HEADER *VariableStore;
  BOOLEAN               FullyInitializeStore;
  EFI_PHYSICAL_ADDRESS  *VariableBase;
  UINTN                 *LastVariableOffset;
  VARIABLE_STORE_HEADER *VariableStoreHeader;
  AUTH_VAR_HEADER       *Variable;
  VOID                  *VariableData;
  UINTN                  ScratchSize;
  EFI_HOB_GUID_TYPE     *GuidHob;
  EFI_GUID              *VariableGuid;

  FullyInitializeStore = TRUE;

  if (VolatileStore) {
    VariableBase = &mVariableModuleGlobal->VariableGlobal.VolatileVariableBase;
    LastVariableOffset = &mVariableModuleGlobal->VolatileLastVariableOffset;
  } else {
    VariableBase = &mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
    LastVariableOffset = &mVariableModuleGlobal->NonVolatileLastVariableOffset;
  }

  //
  // Note that in EdkII variable driver implementation, Hardware Error Record type variable
  // is stored with common variable in the same NV region. So the platform integrator should
  // ensure that the value of PcdHwErrStorageSize is less than or equal to the value of 
  // PcdVariableStoreSize.
  //
  ASSERT (PcdGet32 (PcdHwErrStorageSize) <= PcdGet32 (PcdVariableStoreSize));

  if (VolatileStore) {
    ScratchSize = GetNonVolatileMaxVariableSize ();
    mVariableModuleGlobal->ScratchBufferSize = ScratchSize;
  } else {
    ScratchSize = 0;
  }

  //
  // Allocate memory for variable store.  For volatile memory we also
  // allocate enough scratch space for one variable, to be used for append
  // writes.
  //
  if (VolatileStore || (PcdGet64 (PcdEmuVariableNvStoreReserved) == 0)) {
    VariableStore = (VARIABLE_STORE_HEADER *)
      AllocateRuntimePool (PcdGet32 (PcdVariableStoreSize) + ScratchSize);
  } else {
    //
    // A memory location has been reserved for the NV variable store.  Certain
    // platforms may be able to preserve a memory range across system resets,
    // thereby providing better NV variable emulation.
    //
    VariableStore =
      (VARIABLE_STORE_HEADER *)(VOID*)(UINTN)
        PcdGet64 (PcdEmuVariableNvStoreReserved);
    if (
         (VariableStore->Size == PcdGet32 (PcdVariableStoreSize)) &&
         (VariableStore->Format == VARIABLE_STORE_FORMATTED) &&
         (VariableStore->State == VARIABLE_STORE_HEALTHY)
       ) {
      DEBUG((
        EFI_D_INFO,
        "Variable Store reserved at %p appears to be valid\n",
        VariableStore
        ));
      FullyInitializeStore = FALSE;
    }
  }

  if (NULL == VariableStore) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (FullyInitializeStore) {
    SetMem (VariableStore, PcdGet32 (PcdVariableStoreSize) + ScratchSize, 0xff);
  }

  //
  // Variable Specific Data
  //
  VariableGuid              = &gEfiAuthenticatedVariableGuid;
  *VariableBase             = (EFI_PHYSICAL_ADDRESS) VariableStore;
  InitializeLocationForLastVariableOffset (VariableStore, LastVariableOffset);

  CopyGuid (&VariableStore->Signature, VariableGuid);
  VariableStore->Size       = PcdGet32 (PcdVariableStoreSize);
  VariableStore->Format     = VARIABLE_STORE_FORMATTED;
  VariableStore->State      = VARIABLE_STORE_HEALTHY;
  VariableStore->Reserved   = 0;
  VariableStore->Reserved1  = 0;

  if (!VolatileStore) {
    //
    // Get HOB variable store.
    //
    GuidHob = GetFirstGuidHob (VariableGuid);
    if (GuidHob != NULL) {
      VariableStoreHeader = (VARIABLE_STORE_HEADER *) GET_GUID_HOB_DATA (GuidHob);
      if (CompareGuid (&VariableStoreHeader->Signature, VariableGuid) &&
          (VariableStoreHeader->Format == VARIABLE_STORE_FORMATTED) &&
          (VariableStoreHeader->State == VARIABLE_STORE_HEALTHY)
         ) {
        DEBUG ((EFI_D_INFO, "HOB Variable Store appears to be valid.\n"));
        //
        // Flush the HOB variable to Emulation Variable storage.
        //
        for ( Variable = GetStartPointer (VariableStoreHeader)
            ; (Variable < GetEndPointer (VariableStoreHeader) && (Variable != NULL))
            ; Variable = GetNextVariablePtr (Variable)
            ) {
          ASSERT (Variable->State == VAR_ADDED);
          ASSERT ((Variable->Attributes & EFI_VARIABLE_NON_VOLATILE) != 0);
          VariableData = GetVariableDataPtr (Variable);
          Status = EmuSetVariable (
                     GetVariableNamePtr (Variable),
                     &Variable->VendorGuid,
                     Variable->Attributes,
                     Variable->DataSize,
                     VariableData,
                     &mVariableModuleGlobal->VariableGlobal,
                     &mVariableModuleGlobal->VolatileLastVariableOffset,
                     &mVariableModuleGlobal->NonVolatileLastVariableOffset
                     );
          ASSERT_EFI_ERROR (Status);
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Initializes variable store area for non-volatile and volatile variable.

  This function allocates and initializes memory space for global context of ESAL
  variable service and variable store area for non-volatile and volatile variable.

  @param  ImageHandle           The Image handle of this driver.
  @param  SystemTable           The pointer of EFI_SYSTEM_TABLE.

  @retval EFI_SUCCESS           Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES  Fail to allocate enough memory resource.

**/
EFI_STATUS
EFIAPI
VariableCommonInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Allocate memory for mVariableModuleGlobal
  //
  mVariableModuleGlobal = (VARIABLE_MODULE_GLOBAL *)
    AllocateRuntimeZeroPool (sizeof (VARIABLE_MODULE_GLOBAL));
  if (NULL == mVariableModuleGlobal) {
    return EFI_OUT_OF_RESOURCES;
  }

  EfiInitializeLock(&mVariableModuleGlobal->VariableGlobal.VariableServicesLock, TPL_NOTIFY);

  mVariableModuleGlobal->MaxVariableSize = PcdGet32 (PcdMaxVariableSize);
  mVariableModuleGlobal->MaxAuthVariableSize =
    ((PcdGet32 (PcdMaxAuthVariableSize) != 0) ?
     PcdGet32 (PcdMaxAuthVariableSize) : mVariableModuleGlobal->MaxVariableSize);

  //
  // Intialize volatile variable store
  //
  Status = InitializeVariableStore (TRUE);
  if (EFI_ERROR (Status)) {
    FreePool(mVariableModuleGlobal);
    return Status;
  }
  //
  // Intialize non volatile variable store
  //
  Status = InitializeVariableStore (FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mVariableCommonIsInitialized = TRUE;

  return Status;
}

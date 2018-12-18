/** @file

  Implementation for the EEprom NV store.

  Copyright (c) 2016, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Variable.h"

#define MLNX_VARIABLE_STORE_PREFIX      "NV Store"

///
/// Whether the NV initialization routine has been called.
///
STATIC BOOLEAN mNvStoreIsInitialized;

///
/// Address of the NV Store on the storage medium.
///
STATIC UINT32 mNvStoreAddr;

///
/// The I2C EEPROM interface from the Bluefield EEPROM protocol used
/// for the NV variable store.
///
BLUEFIELD_EEPROM_PROTOCOL   mI2cEeprom;

STATIC
EFI_STATUS
LocateBluefieldI2cEepromProtocol ()
{
  BLUEFIELD_EEPROM_PROTOCOL *I2cEeprom;
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gBluefieldEepromProtocolGuid,
                                 NULL,
                                (VOID **) &I2cEeprom);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR,
            "Failed to locate Bluefield I2cEeprom protocol, abort!\n"));
    return EFI_ABORTED;
  }

  ASSERT (I2cEeprom           != NULL);
  ASSERT (I2cEeprom->Transfer != NULL);
  mI2cEeprom  = *I2cEeprom;

  return EFI_SUCCESS;
}

/**
  Copy the NV store from EEProm into memory.  The work in this
  function is done exactly once, after VariableCommonInitialize has
  completed.

**/
BOOLEAN
InitNvStore ()
{
  EFI_STATUS                Status;
  VARIABLE_STORE_HEADER     *NvHeader;
  VOID                      *NvStore;
  UINT8                     *Store;
  UINTN                     StoreSize;
  UINTN                     HeaderSize;

  ASSERT (mVariableCommonIsInitialized);

  if (mNvStoreIsInitialized) {
    return TRUE;
  }

  mNvStoreIsInitialized = TRUE;

  //
  // Find the Bluefield Eeprom protocol. Currently we expect only one but if
  // there are multiple we need to further disambiguate.
  //
  Status = LocateBluefieldI2cEepromProtocol ();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR,
            "InitNvStore: Cannot find Bluefield I2cEeprom Protocol.\n"));
    return FALSE;
  }

  //
  // Allocate a temporary buffer large enough to hold all the
  // variables if the variable store header is valid.
  //
  StoreSize = PcdGet32 (PcdVariableStoreSize);
  Store     = (UINT8 *) AllocatePool (StoreSize + 1);
  ASSERT (Store != NULL);
  SetMem (Store, StoreSize, 0xff);

  HeaderSize = sizeof (VARIABLE_STORE_HEADER);
  NvHeader = (VARIABLE_STORE_HEADER *) Store;

  //
  // Look for a valid store from EEProm.
  //
  mNvStoreAddr = 0;
  Status = mI2cEeprom.Transfer (&mI2cEeprom, mNvStoreAddr, HeaderSize,
                                (UINT8 *) NvHeader, EEPROM_READ);
  ASSERT_EFI_ERROR (Status);

  if (GetVariableStoreStatus (NvHeader) != EfiValid) {
    mNvStoreAddr += StoreSize;
    Status = mI2cEeprom.Transfer (&mI2cEeprom, mNvStoreAddr, HeaderSize,
                                  (UINT8 *) NvHeader, EEPROM_READ);
    ASSERT_EFI_ERROR (Status);
  }

  NvStore =
    (VOID *)mVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;

  if (GetVariableStoreStatus (NvHeader) == EfiValid) {
    //
    // Read NvStore from EEProm.
    //
    DEBUG ((EFI_D_VARIABLE, "InitNvStore: read from EEProm\n"));
    Status = mI2cEeprom.Transfer (&mI2cEeprom, mNvStoreAddr, StoreSize,
                                    Store, EEPROM_READ);
    ASSERT_EFI_ERROR (Status);
    //
    // Verify the variables the store contains. Also enable the auto-
    // recovery, so we do not lose the existant valid variables. This
    // function does not restore the actual content of the store but
    // does the necessary to keep the system working properly.
    // Note that we do not expect EFI_INVALID_PARAMETER error since the
    // header has been verified and is valid. Thus handle the remaining
    // error codes.
    //
    Status = VariableStoreCheckAndRestore (MLNX_VARIABLE_STORE_PREFIX,
                                            Store, TRUE);
    if (Status != EFI_NOT_FOUND) {
      //
      // No default variables are missing; either the store is valid
      // or all the valid variables has been reinstalled. So look for
      // the last variable offset and copy the store content to the
      // variable base.
      //
      InitializeLocationForLastVariableOffset (NvHeader,
                    &mVariableModuleGlobal->NonVolatileLastVariableOffset);
      CopyMem (NvStore, Store, StoreSize);
    }

    if (EFI_ERROR (Status)) {
        //
        // The variable store content is corrupted. Thus, invalidate the
        // NvHeader to force the initialization of the store; see statement
        // below.
        //
        SetMem (Store, HeaderSize, 0xff);
    }
  }

  if (GetVariableStoreStatus (NvHeader) != EfiValid) {
    //
    // Initialize the contents of EEProm.
    //
    DEBUG ((EFI_D_VARIABLE, "InitNvStore: initializing EEProm\n"));

    mNvStoreAddr = 0;
    Status = mI2cEeprom.Transfer (&mI2cEeprom, mNvStoreAddr, StoreSize,
                                    (UINT8 *) NvStore, EEPROM_WRITE);
    ASSERT_EFI_ERROR (Status);
  }

  FreePool (Store);

  PrintVariableStore (MLNX_VARIABLE_STORE_PREFIX, NvStore);

  return TRUE;
}

/**
  Return whether the EEProm store is active.

**/
BOOLEAN
IsActiveNvStore ()
{
  return mI2cEeprom.Transfer != NULL;
}

/**
  Flush data to the active NV store in Smbus EEProm.

  @param  Offset         The offset to the NV store.
  @param  Data           The data to write.
  @param  DataSize       The size of the data in bytes.

**/
EFI_STATUS
FlushToNvStore (
  IN  UINT32  Offset,
  IN  UINT8  *Data,
  IN  UINTN   DataSize
  )
{
  EFI_STATUS  Status;

  ASSERT (Offset + DataSize <= PcdGet32 (PcdVariableStoreSize));
  Status = mI2cEeprom.Transfer (&mI2cEeprom, (mNvStoreAddr + Offset),
                                DataSize, (UINT8 *) Data, EEPROM_WRITE);
  return Status;
}

/**
  Commit the result of a reclaiming operation to the NV store in Smbus
  EEProm.  This requires: (1) write the new store data to inactive
  store, then make it active.  (2) invalidate the current active
  store.

  @param  Store          The Store data to write.
  @param  StoreSize      The size of the store data.

**/
EFI_STATUS
ReclaimNvStore (
  IN  UINT8  *Store,
  IN  UINTN   StoreSize
  )
{
  EFI_STATUS                Status;
  UINT32                    OldStoreAddr;
  UINT32                    NewStoreAddr;
  VARIABLE_STORE_HEADER     Header;
  UINTN                     HeaderSize;

  OldStoreAddr = mNvStoreAddr;
  NewStoreAddr = PcdGet32 (PcdVariableStoreSize) - mNvStoreAddr;

  Status = mI2cEeprom.Transfer (&mI2cEeprom, NewStoreAddr, StoreSize,
                                (UINT8 *) Store, EEPROM_WRITE);
  if (EFI_ERROR (Status))
    return Status;

  HeaderSize = sizeof (VARIABLE_STORE_HEADER);
  SetMem (&Header, HeaderSize, 0xff);
  Status = mI2cEeprom.Transfer (&mI2cEeprom, OldStoreAddr, HeaderSize,
                                (UINT8 *) &Header, EEPROM_WRITE);
  return Status;
}

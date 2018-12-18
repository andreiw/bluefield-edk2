/** @file

  The internal header file includes definitions for the EEprom NV
  store implementation.

  Copyright (c) 2016, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _SMBUS_EEPROM_NV_STORE_H_
#define _SMBUS_EEPROM_NV_STORE_H_

/**
  Copy the NV store from EEProm into memory.  The work in this
  function is done exactly once, after VariableCommonInitialize has
  completed.

**/
BOOLEAN
InitNvStore ();

/**
  Return whether the EEProm store is active.

**/
BOOLEAN
IsActiveNvStore ();

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
  );

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
  );

#endif

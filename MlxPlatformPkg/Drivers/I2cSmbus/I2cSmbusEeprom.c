/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

// I2cSmbusEeprom driver creates BLUEFIELD_EEPROM_PROTOCOL, which is used for
// managing eeprom.
//
// The BLUEFIELD_EEPROM_PROTOCOL is one typical consumer of the SMBus Host
// Protocol. It allows the device drivers to start I2C SMBus transactions
// through the I2cSmbusExecute function. This function initiates an SMBus
// transaction on the SMBus Host Controller. The I2cSmbusExecute API requires
// that the I2C SMbus is in the correct configuration for the transaction.

#include "I2cSmbusRuntimeDxe.h"
#include "I2cSmbusEeprom.h"

// EEPROM global context.
EEPROM_CONTEXT gEepromContext;

// I2C-based request packet.
STATIC I2C_SMBUS_REQUEST_PACKET mRequestPacket;

// Return the address width in bytes. It is used to determine the number
// of 8-bit words needed to address the content of the EEPROM.
#define ADDRESS_WIDTH(size)     (4 - (__builtin_clz((size) - 1) >> 3))

// Initiate an EEPROM transfer through the SMBus Host Protocol.
//
// EEPROM transactions are based on two main operations: writing the memory
// index first, then reading or writing data bytes from memory. If W-W or W-R
// cycles are defined, then internal address should be set before writing or
// reading data bytes.
// Note that this transfer uses the I2C_REQUEST_PACKET that encapsulates the
// transaction information which allow the bus controller to perform SMBus
// cycles. Also note that EEPROM transaction defines MAX_PAGE_SIZE bytes per
// operation -i.e the allowed number per block transfer.
EFI_STATUS
EFIAPI
I2cSmbusEepromTransfer (
  IN CONST BLUEFIELD_EEPROM_PROTOCOL *This,
  IN UINT16                          Address,
  IN UINT32                          Length,
  IN UINT8                           *Buffer,
  IN UINT8                           Operation
  )
{
  EFI_SMBUS_DEVICE_ADDRESS  DeviceAddress;
  EFI_SMBUS_OPERATION       SmbusOperation;
  UINTN                     BufferLength;
  UINT16                    PageSize, PageOffset;
  BOOLEAN                   PecEnable;
  UINT32                    Transmitted    = 0;
  UINT32                    CurrentAddress = Address;
  UINT32                    OpIdx          = 0;
  EFI_STATUS                Status         = EFI_SUCCESS;

  // Set Smbus slave address.
  DeviceAddress.SmbusDeviceAddress = (UINT8) gEepromContext.Chip;

  PageSize  = gEepromContext.PageSize;
  PecEnable = gEepromContext.PecEnable;

  mRequestPacket.OperationCount = 1;
  // The master might use a combined W-R cycle to the slave. It can also do
  // a W-W. The "Write followed by Read" operation sequence is by far one of
  // the most common implementation. It is typically used to send a command to
  // a device and then read data based on the command sent.
  // For instance, the W-R operations could be used to read from EEPROMs. The
  // initial write tells the EEPROM of the specific offset to be read from in
  // the subsequent read operation. Note that operation count is set to 1
  // when EEPROM_WRITE is set because the underlying Mellanox SMBus hardware is
  // able to perform the W-W operation through a single command. Unlike the W-W
  // the combined R-W requires two separate operations.
  if (Operation == EEPROM_READ) {
    mRequestPacket.Operation[OpIdx].LengthInBytes = 0;
    mRequestPacket.OperationCount++;
    OpIdx++;
  }

  mRequestPacket.Operation[OpIdx].Flags =
                    (Operation == EEPROM_READ) ? I2C_FLAG_READ : 0;

  // Set the particular SMBus protocol instance it will use to execute the
  // SMBus transactions.
  SmbusOperation =
          (Operation == EEPROM_READ) ? EfiSmbusReadBlock : EfiSmbusWriteBlock;

  while (Length > 0) {
    // Set transaction control buffer - i.e. internal device address.
    CurrentAddress  = (Address + Transmitted) % gEepromContext.Size;
    // Set transaction data buffer length.
    //
    // Note that the EEPROM device used to store system variables supports
    // a "page write" mode. In "page write" mode, the data byte address lower
    // bits (corresponding to the page mask (= PageSize - 1)) are internally
    // incremented following the receipt of each data byte. The higher data
    // byte address bits are not incremented, retaining the memory page row
    // location. When the byte address, internally generated, reaches the page
    // boundary, the following byte is placed at the beginning of the same page,
    // i.e. the address roll over is from the last byte of the current page to
    // the first byte of the same page. If more than PAGE_SIZE data bytes are
    // transmitted to the EEPROM, the data word address will "roll over" and
    // previous data will be overwritten. In order to recreate a sequential
    // write, we have to trancate writes at page boundaries.
    if (Operation != EEPROM_READ) {
        PageOffset   = CurrentAddress & (PageSize - 1);
        BufferLength = ((PageOffset + Length) <= PageSize) ? \
                    Length : (PageSize - PageOffset);
    } else {
        // Note that read transactions are not limited by a "page" mode, but
        // offer full sequential semantics. As a result, we don't have to
        // truncate reads at page boundaries. Note that when the memory address
        // limit is reached, the data byte address will roll over and the
        // sequential read will continue.
        BufferLength = Length;
    }

    mRequestPacket.Operation[OpIdx].Buffer         = Buffer + Transmitted;
    mRequestPacket.Operation[OpIdx].LengthInBytes  = BufferLength;

    // Executes an SMBus operation to an SMBus controller.
    Status = gEepromContext.I2cSmbusExecute (
                                NULL,
                                DeviceAddress,
                                CurrentAddress,
                                SmbusOperation,
                                PecEnable,
                                &BufferLength,
                               (VOID *) &mRequestPacket
                               );
    if (EFI_ERROR(Status))
      break;

    Length      -= BufferLength;
    Transmitted += BufferLength;
  }

  return Status;
}

EFI_STATUS
EFIAPI
I2cSmbusInstallEepromProtocol (
  VOID
  )
{
  EFI_STATUS      Status = EFI_SUCCESS;
  UINT8           Bus;
  UINT8           Chip;
  UINT8           AddressWidth;
  UINT8           PageSize;
  INT32           Size;

  ASSERT (gSmbusContext.SmbusHc.Execute != NULL);

  // Read EEPROM address in the I2C bus and config parameters.
  Bus          = PcdGet8 (PcdEepromBusId);
  Chip         = PcdGet8 (PcdEepromStoreAddress);
  Size         = PcdGet32 (PcdEepromStoreSize);
  PageSize     = PcdGet32 (PcdEepromStorePageSize);
  AddressWidth = ADDRESS_WIDTH (Size);

  gEepromContext.Signature               = EEPROM_SIGNATURE;
  gEepromContext.EepromProtocol.Transfer = I2cSmbusEepromTransfer;
  gEepromContext.I2cSmbusExecute         = gSmbusContext.SmbusHc.Execute;
  gEepromContext.Chip                    = I2C_DEVICE_ADDRESS(Chip);
  gEepromContext.Size                    = Size;
  gEepromContext.AddressWidth            = AddressWidth;
  gEepromContext.PageSize                = PageSize;
  gEepromContext.PecEnable               = EEPROM_PEC_CHECK_EN;

  Status = I2cSmbusRegisterSlaveDevice (Bus, Chip);
  if (EFI_ERROR (Status))
    goto out;

  Status = gBS->InstallMultipleProtocolInterfaces (
      &gEepromContext.ControllerHandle,
      &gBluefieldEepromProtocolGuid,
      &gEepromContext.EepromProtocol,
      NULL
      );

out:
  DEBUG ((DEBUG_ERROR,
        "I2cSmbusInstallEepromProtocol: %r\n", Status));
  return Status;
}

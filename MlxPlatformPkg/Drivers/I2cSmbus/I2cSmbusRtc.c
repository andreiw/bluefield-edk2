/** @file
  Implement EFI RealTimeClock runtime services.

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cSmbusRuntimeDxe.h"
#include "I2cSmbusRtc.h"

RTC_CONTEXT gRtcContext;

STATIC BOOLEAN mRtcInitialized = FALSE;

STATIC I2C_SMBUS_REQUEST_PACKET mRequestPacket;

// Initiate an Rtc transfer through the SMBus Host Protocol.
//
// RTC transactions are based on two main operations: writing the register/RAM
// address first, then reading or writing data bytes from regiters/RAM.
// Note that this transfer uses the I2C_REQUEST_PACKET that encapsulates the
// transaction information which allow the bus controller to perform SMBus
// cycles.
EFI_STATUS
EFIAPI
I2cSmbusRtcTransfer (
  IN UINT16  Address,
  IN UINT8   Length,
  IN UINT8  *Buffer,
  IN UINT8   Operation
  )
{
  EFI_SMBUS_DEVICE_ADDRESS  DeviceAddress;
  EFI_SMBUS_OPERATION       SmbusOperation;
  BOOLEAN                   PecEnable;
  UINT32                    OpIdx        = 0;
  UINTN                     BufferLength = Length;
  EFI_STATUS                Status       = EFI_SUCCESS;

  // Set Smbus slave address.
  DeviceAddress.SmbusDeviceAddress = (UINT8) gRtcContext.Chip;

  PecEnable = gRtcContext.PecEnable;

  mRequestPacket.OperationCount = 1;
  // The master might use a combined W-R cycle to the slave. The "Write
  // followed by Read" is typically used to send a command to a device and
  // then read data based on the command sent.
  // For instance, the W-R operations could be used to read RTC registers.
  // The initial write tells the RTC of the register address to be read from
  // in the subsequent read operation. Note that operation count is set to 1
  // when EEPROM_WRITE is set because the underlying Mellanox SMBus hardware
  // is able to perform the W-W operation through a single command. Unlike
  // the W-W the combined R-W requires two separate operations.
  if (Operation == RTC_READ) {
    mRequestPacket.Operation[OpIdx].LengthInBytes = 0;
    mRequestPacket.OperationCount++;
    OpIdx++;
  }

  mRequestPacket.Operation[OpIdx].Flags =
                    (Operation == RTC_READ) ? I2C_FLAG_READ : 0;

  // Set the particular SMBus protocol instance it will use to execute the
  // SMBus transactions.
  SmbusOperation =
          (Operation == RTC_READ) ? EfiSmbusReadBlock : EfiSmbusWriteBlock;

  if (BufferLength > RTC_MAX_BUFFER_LENGTH)
    return EFI_BAD_BUFFER_SIZE;

  mRequestPacket.Operation[OpIdx].Buffer         = Buffer;
  mRequestPacket.Operation[OpIdx].LengthInBytes  = BufferLength;

  // Executes an SMBus operation to an SMBus controller.
  Status = gRtcContext.I2cSmbusExecute (
                              NULL,
                              DeviceAddress,
                              Address,
                              SmbusOperation,
                              PecEnable,
                              &BufferLength,
                              (VOID *) &mRequestPacket
                              );

    return Status;
}

EFI_STATUS
I2cSmbusRtcGetTime (IN CONST BLUEFIELD_RTC_PROTOCOL *This,
  IN OUT UINT16 *Year,
  IN OUT UINT8  *Month,
  IN OUT UINT8  *Day,
  IN OUT UINT8  *Hour,
  IN OUT UINT8  *Minute,
  IN OUT UINT8  *Second
  )
{
  RTC_DS1338_REGS *RtcRegs;
  EFI_STATUS      Status;
  UINT8           BaseHour;
  UINT16          BaseYear = 1900;
  UINT8           Temp [DS1338_REGS_NUM] = { 0 };

  if (!mRtcInitialized) {
    // Never get here, because mRtcInitialized must be TRUE, otherwise
    // the RTC protocol install must have failed.
    return EFI_NOT_READY;
  }

  Status = I2cSmbusRtcTransfer (
                    0,
                    DS1338_REGS_NUM,
                    Temp,
                    RTC_READ
                    );

  if (EFI_ERROR(Status))
    return Status;

  RtcRegs = (RTC_DS1338_REGS *) Temp;

  // If clock is halted, then turn it on, so clock can tick.
  if (RtcRegs->Seconds.Ch) {
    RtcRegs->Seconds.Ch = 0;

    Status = I2cSmbusRtcTransfer (
                      DS1338_REGADDR_SECONDS,
                      1,
                      &RtcRegs->Seconds.reg,
                      RTC_WRITE
                      );

    if (EFI_ERROR(Status))
      return Status;
  }

  // Verify whether the Oscillator Stop Flag (OSF) is set, then clear it,
  // and warn.
  if (RtcRegs->Control.Osf) {
    RtcRegs->Control.Osf = 0;
    DEBUG ((DEBUG_WARN,
            "I2cSmbusRtcGetTime: Oscillator has stopped or was stopped\n"));

    Status |= I2cSmbusRtcTransfer (
                      DS1338_REGADDR_CONTROL,
                      1,
                      &RtcRegs->Control.reg,
                      RTC_WRITE
                      );

    // Note that clock is invalid, then read the RTC again
    Status |= I2cSmbusRtcTransfer (
                      0,
                      DS1338_REGS_NUM,
                      Temp,
                      RTC_READ
                      );

    if (EFI_ERROR(Status))
      return Status;
  }

  *Year  = BaseYear;
  *Year += (RtcRegs->Year.Year10   * 10) + RtcRegs->Year.Year;
  *Month = (RtcRegs->Month.Month10 * 10) + RtcRegs->Month.Month;
  *Day   = (RtcRegs->Date.Date10   * 10) + RtcRegs->Date.Date;

  BaseHour = 0;
  if (RtcRegs->Hours.PM_20Hours) {
    if (RtcRegs->Hours.Hour24_n)
      return EFI_DEVICE_ERROR;
    BaseHour = 20;
  } else if (RtcRegs->Hours.Hours10) {
    BaseHour = 10;
  }

  *Hour = BaseHour + RtcRegs->Hours.Hour;

  *Minute  = RtcRegs->Minutes.Minutes10 * 10;
  *Minute += RtcRegs->Minutes.Minutes;

  *Second  = RtcRegs->Seconds.Seconds10 * 10;
  *Second += RtcRegs->Seconds.Seconds;

  return EFI_SUCCESS;
}

EFI_STATUS
I2cSmbusRtcSetTime (
  IN CONST BLUEFIELD_RTC_PROTOCOL *This,
  IN UINT16  Year,
  IN UINT8   Month,
  IN UINT8   Day,
  IN UINT8   Hour,
  IN UINT8   Minute,
  IN UINT8   Second
  )
{
  RTC_DS1338_REGS RtcRegs = { 0 };
  UINT16          BaseYear = 1900;
  EFI_STATUS      Status;

  if (!mRtcInitialized) {
    // Never get here, because mRtcInitialized must be TRUE, otherwise
    // the RTC protocol install must have failed.
    return EFI_NOT_READY;
  }

  RtcRegs.Seconds.Seconds10 = Second / 10;
  RtcRegs.Seconds.Seconds   = Second % 10;
  RtcRegs.Seconds.Ch        = 0; // Turn the clock on, so it can tick

  RtcRegs.Minutes.Minutes10 = Minute / 10;
  RtcRegs.Minutes.Minutes   = Minute % 10;

  if (Hour > 19)
    RtcRegs.Hours.PM_20Hours = 1;
  else if (Hour > 9)
    RtcRegs.Hours.Hours10    = 1;

  RtcRegs.Hours.Hour    = Hour % 10;

  RtcRegs.Date.Date10   = Day / 10;
  RtcRegs.Date.Date     = Day % 10;

  RtcRegs.Month.Month10 = Month / 10;
  RtcRegs.Month.Month   = Month % 10;

  RtcRegs.Year.Year10   = (Year - BaseYear) / 10;
  RtcRegs.Year.Year     = (Year - BaseYear) % 10;

  Status = I2cSmbusRtcTransfer (
            0,
            DS1338_REGS_NUM,
            (UINT8 *) &RtcRegs,
            RTC_WRITE
            );

  return Status;
}

STATIC
EFI_STATUS
I2cSmbusInitializeRtc (
  VOID
  )
{
  RTC_DS1338_CONTROL  Control;
  RTC_DS1338_HOURS    Hours;
  EFI_STATUS          Status;

  Status  = I2cSmbusRtcTransfer (
                    DS1338_REGADDR_HOURS,
                    1,
                    &Hours.reg,
                    RTC_READ);

  Status |= I2cSmbusRtcTransfer (
                    DS1338_REGADDR_CONTROL,
                    1,
                    &Control.reg,
                    RTC_READ
                    );

  if (EFI_ERROR (Status))
    return Status;

  // Enable the 12-hour mode.
  Hours.Hour24_n = RTC_12_24_HOUR_EN;

  Control.Osf    = 0; // This bit can only be written to logic 0.
  // Configure the Square-Wave Output to 32.768kHz
  Control.Sqwe   = 1;
  Control.Rs0    = 1;
  Control.Rs1    = 1;

  Status |= I2cSmbusRtcTransfer (
                    DS1338_REGADDR_HOURS,
                    1,
                    &Hours.reg,
                    RTC_WRITE);

  Status |= I2cSmbusRtcTransfer (
                    DS1338_REGADDR_CONTROL,
                    1,
                    &Control.reg,
                    RTC_WRITE
                    );

  if (EFI_ERROR(Status))
    return Status;

  mRtcInitialized = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
I2cSmbusInstallRtcProtocol (
  VOID
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT8       Bus;
  UINT8       Chip;

  ASSERT (gSmbusContext.SmbusHc.Execute != NULL);

  // Read RTC address in the I2C bus and config parameters.
  Chip = PcdGet8 (PcdRtcAddress);
  Bus  = PcdGet8 (PcdRtcBusId);

  gRtcContext.Signature            = RTC_SIGNATURE;
  gRtcContext.RtcProtocol.GetTime  = I2cSmbusRtcGetTime;
  gRtcContext.RtcProtocol.SetTime  = I2cSmbusRtcSetTime;
  gRtcContext.I2cSmbusExecute      = gSmbusContext.SmbusHc.Execute;
  gRtcContext.Chip                 = I2C_DEVICE_ADDRESS(Chip);
  gRtcContext.PecEnable            = RTC_PEC_CHECK_EN;

  Status = I2cSmbusRegisterSlaveDevice (Bus, Chip);
  if (EFI_ERROR (Status))
    goto out;

  // Initialize the RTC
  Status = I2cSmbusInitializeRtc ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,
        "I2cSmbusInstallRtcProtocol: Failed to initilaize the RTC DS1338\n"));
    goto out;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
      &gRtcContext.ControllerHandle,
      &gBluefieldRtcProtocolGuid,
      &gRtcContext.RtcProtocol,
      NULL
      );

out:
  DEBUG ((DEBUG_ERROR,
        "I2cSmbusInstallRtcProtocol: %r\n", Status));
  return Status;
}

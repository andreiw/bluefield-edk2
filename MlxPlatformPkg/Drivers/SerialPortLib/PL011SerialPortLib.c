/** @file
  Serial I/O Port library functions with no library constructor/destructor

  Copyright (c) 2008 - 2010, Apple Inc. All rights reserved.<BR>
  Copyright (c) 2012 - 2013, ARM Ltd. All rights reserved.<BR>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>
#include <Library/SerialPortExtLib.h>

#include <Drivers/PL011Uart.h>

#include "BlueFieldPlatform.h"
#include "../TmFifoDxe/TmFifoLib.h"

/**

  Programmed hardware of Serial port.

  @return    Always return RETURN_UNSUPPORTED.

**/
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  UINT64              BaudRate;
  UINT32              ReceiveFifoDepth;
  EFI_PARITY_TYPE     Parity;
  UINT8               DataBits;
  EFI_STOP_BITS_TYPE  StopBits;
  RETURN_STATUS       Result0, Result1;

  BaudRate = (UINTN)PcdGet64 (PcdUartDefaultBaudRate);
  ReceiveFifoDepth = 0; // Use the default value for Fifo depth
  Parity = (EFI_PARITY_TYPE)PcdGet8 (PcdUartDefaultParity);
  DataBits = PcdGet8 (PcdUartDefaultDataBits);
  StopBits = (EFI_STOP_BITS_TYPE) PcdGet8 (PcdUartDefaultStopBits);

  Result0 = PL011UartInitializePort (BLUEFIELD_UART0_BASE, &BaudRate,
      &ReceiveFifoDepth, &Parity, &DataBits, &StopBits);

  Result1 = PL011UartInitializePort (BLUEFIELD_UART1_BASE, &BaudRate,
      &ReceiveFifoDepth, &Parity, &DataBits, &StopBits);

  if (Result0 == EFI_SUCCESS || Result1 == EFI_SUCCESS) {
    return EFI_SUCCESS;
  } else {
    return Result0;
  }
}

/**
  Write data to serial device.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes written to serial device.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
  )
{
  UINTN Len, Max = 0;

  // Write to all UART and return the maximum length.

  Len = PL011UartWrite (BLUEFIELD_UART0_BASE, Buffer, NumberOfBytes);
  if (Len > Max) {
    Max = Len;
  }

  Len = PL011UartWrite (BLUEFIELD_UART1_BASE, Buffer, NumberOfBytes);
  if (Len > Max) {
    Max = Len;
  }

  // Write to TMFIFO console.
  TmFifoConsWrite(Buffer, NumberOfBytes);

  return Max;
}

/**
  Read data from serial device and save the data in buffer.

  @param  Buffer           Point of data buffer which need to be written.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Actual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  UINTN  BytesLeft = NumberOfBytes;

  while (BytesLeft > 0) {
    while (PL011UartPoll (BLUEFIELD_UART0_BASE) && BytesLeft > 0) {
      PL011UartRead (BLUEFIELD_UART0_BASE, Buffer++, 1);
      BytesLeft--;
    }
    while (PL011UartPoll (BLUEFIELD_UART1_BASE) && BytesLeft > 0) {
      PL011UartRead (BLUEFIELD_UART1_BASE, Buffer++, 1);
      BytesLeft--;
    }
    while (TmFifoConsPoll () && BytesLeft > 0) {
      TmFifoConsRead (Buffer++, 1);
      BytesLeft--;
    }
  }

  return NumberOfBytes;
}

/**
  Check to see if any data is available to be read from the debug device.

  @retval EFI_SUCCESS       At least one byte of data is available to be read
  @retval EFI_NOT_READY     No data is available to be read
  @retval EFI_DEVICE_ERROR  The serial device is not functioning properly

**/
BOOLEAN
EFIAPI
SerialPortPoll (
  VOID
  )
{
  return PL011UartPoll (BLUEFIELD_UART0_BASE) ||
         PL011UartPoll (BLUEFIELD_UART1_BASE) ||
         TmFifoConsPoll ();
}

/** @file

  Copyright (c) 2017, Mellanox Technologies. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD
  License which accompanies this distribution.  The full text of the license
  may be found at http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_SMBUS_RTC_DS1338_H__
#define __I2C_SMBUS_RTC_DS1338_H__

#include <Uefi.h>

#include <Protocol/SmbusHc.h>
#include <Protocol/Rtc.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <PiDxe.h>

// RTC and RAM Address Map
#define DS1338_REGADDR_SECONDS      0x00
#define DS1338_REGADDR_MIUTES       0x01
#define DS1338_REGADDR_HOURS        0x02
#define DS1338_REGADDR_DAY          0x03
#define DS1338_REGADDR_DATE         0x04
#define DS1338_REGADDR_MONTH        0x05
#define DS1338_REGADDR_YEAR         0x06
#define DS1338_REGADDR_CONTROL      0x07

#define DS1338_RAMADDR_START        0x08
#define DS1338_RAMADDR_END          0x3F

#define DS1338_REGS_NUM             8

typedef union {
  struct {
    UINT8 Seconds   : 4;
    UINT8 Seconds10 : 3;
    UINT8 Ch        : 1; // Clock Halt, if 0 then the oscillator is enabled.
  };
  UINT8 reg;
} RTC_DS1338_SECONDS;

#define RTC_DS1338_BIT_CH   0x80

typedef union {
  struct {
    UINT8 Minutes   : 4;
    UINT8 Minutes10 : 3;
    UINT8 Rsv       : 1;
  };
  UINT8 reg;
} RTC_DS1338_MINUTES;

typedef union {
  struct {
    UINT8 Hour       : 4;
    UINT8 Hours10    : 1;
    UINT8 PM_20Hours : 1;
    UINT8 Hour24_n   : 1;
    UINT8 Rsv        : 1;
  };
  UINT8 reg;
} RTC_DS1338_HOURS;

typedef union {
  struct {
    UINT8 Day : 3;
    UINT8 Rsv : 5;
  };
  UINT8 reg;
}RTC_DS1338_DAY;

typedef union {
  struct {
    UINT8 Date   : 4;
    UINT8 Date10 : 2;
    UINT8 Rsv    : 2;
  };
  UINT8 reg;
}RTC_DS1338_DATE;

typedef union {
  struct {
    UINT8 Month   : 4;
    UINT8 Month10 : 1;
    UINT8 Rsv     : 3;
  };
  UINT8 reg;
}RTC_DS1338_MONTH;

typedef union {
  struct {
    UINT8 Year   : 4;
    UINT8 Year10 : 4;
  };
  UINT8 reg;
}RTC_DS1338_YEAR;

typedef union {
  struct {
    UINT8 Rs0  : 1; // Rate Select 0
    UINT8 Rs1  : 1; // Rate Select 1
    UINT8 Rsv0 : 2; // Reserved - must be 0
    UINT8 Sqwe : 1; // Square-Wave Enable
    UINT8 Osf  : 1; // Oscillator Stop Flag
    UINT8 Rsv1 : 1; // Reserved - must be 0
    UINT8 Out  : 1; // Output Control
  };
  UINT8 reg;
} RTC_DS1338_CONTROL;

#define RTC_DS1338_BIT_OSF  0x20

typedef struct {
  RTC_DS1338_SECONDS Seconds;
  RTC_DS1338_MINUTES Minutes;
  RTC_DS1338_HOURS   Hours;
  RTC_DS1338_DAY     Day;
  RTC_DS1338_DATE    Date;
  RTC_DS1338_MONTH   Month;
  RTC_DS1338_YEAR    Year;
  RTC_DS1338_CONTROL Control;
} RTC_DS1338_REGS;

#define RTC_SIGNATURE       SIGNATURE_32 ('R', 'T', 'C', 'M')

// Defines the RTC transfer operation type.
#define RTC_READ          0x1
#define RTC_WRITE         0x0

// Defines whether to use packet error code (PEC).
#define RTC_PEC_CHECK_EN    FALSE

// Defines whether the 12/24-hour mode is enabled.
//  '0' : 24-hour mode
//  '1' : 12-hour mode
#define RTC_12_24_HOUR_EN   0

// Defines the number of bytes allowed per transaction. We set the maximum
// number of bytes with regards to the size of the RTC device address space.
#define RTC_MAX_BUFFER_LENGTH   64 // RTC: 8 bytes - RAM: 56 bytes

// Encapsulates RTC context information.
typedef struct {
  UINT32                         Signature;
  EFI_HANDLE                     ControllerHandle;
  INTN                           Chip;
  UINT8                          AddressWidth;
  BOOLEAN                        PecEnable;
  EFI_SMBUS_HC_EXECUTE_OPERATION I2cSmbusExecute;
  BLUEFIELD_RTC_PROTOCOL         RtcProtocol;
} RTC_CONTEXT;

extern RTC_CONTEXT gRtcContext;

// Returns the current time and date information of the hardware platform.
EFI_STATUS
I2cSmbusRtcGetTime (
  IN CONST BLUEFIELD_RTC_PROTOCOL *This,
  IN OUT UINT16  *Year,
  IN OUT UINT8   *Month,
  IN OUT UINT8   *Day,
  IN OUT UINT8   *Hour,
  IN OUT UINT8   *Minute,
  IN OUT UINT8   *Second
  );

// Sets the current local time and date information.
EFI_STATUS
I2cSmbusRtcSetTime (
  IN CONST BLUEFIELD_RTC_PROTOCOL *This,
  IN UINT16  Year,
  IN UINT8   Month,
  IN UINT8   Day,
  IN UINT8   Hour,
  IN UINT8   Minute,
  IN UINT8   Second
  );

EFI_STATUS
I2cSmbusInstallRtcProtocol (
  VOID
  );

#endif  // __I2C_SMBUS_RTC_DS1338_H__

